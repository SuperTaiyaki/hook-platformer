#!/usr/bin/python2

# vim: ts=4 sw=4 et

import pygame
import sys
import math
import random

pygame.init()

bg = pygame.Color(0x80808000)
fg = pygame.Color(0xFFFFFFFF)
mg = pygame.Color(0xff0000ff)
rg = pygame.Color(0x0000ffff)
color2 = pygame.Color(0x00ff00ff)
black = pygame.Color(0x0)
GRAVITY=-50.0
STRETCH_FACTOR=200.0
FIELD_HEIGHT=2000
ROPE_LENGTH = 600
FIELD_WIDTH = 3000

dots = []

class Hook(object):
    def __init__(self, pos, world):
        self.origin = pos
        self.length = 0
        self.pull = False
        self.stuck = False
        self.release = False
        self.nodes = []
        self.idle = True # Cable not fired
        self.world = world

    def fire(self, dir):
        self.idle = False
        self.dir = [dir[0], dir[1]]
        self.pos = [self.origin.pos[0], self.origin.pos[1]]
        self.nodes = []
        self.node_angles = []
        self.release = True # Launch stage, don't delete the hook
        self.pull = False # Retracting cable (RMB)
        self.stuck = False # Hook has latched onto something
        self.angle_hook = 0
        self.angle_origin = 0
        self.pull_force = 7000
        self.slack_length = 0
        self.real_length = 0 # tension factor
        self.brake = False # Allow more cable to pull? (one way, always allows cable to shrink)

    def last_node(self):
        if not self.nodes:
            return self.origin.pos
        return self.nodes[-1]
    def first_node(self):
        if not self.nodes:
            return self.pos
        return self.nodes[0]

    def update(self, ts):
        if self.idle:
            return
        if not self.stuck:
            self.dir[1] -= GRAVITY * ts
            #print "Before: %f %f" % (self.dir[0], self.dir[1])
            # This is going to do weird things at low framerate
            if self.pull or self.brake:

                # Calculate the player's outwards velocity first

                fn = self.first_node()
                vec_player = (self.origin.pos[0] - fn[0], self.origin.pos[1] - fn[1]) # unscaled

                p_outward_component = outward_speed(vec_player, self.origin.velocity)
                if p_outward_component < 0:
                    p_outward_component = 0

                ### Dodgy system that kills outwards force
                # Lock the cable - don't let it grow outwards
                ln = self.last_node()
                cable_len = math.hypot(self.pos[0]-ln[0], self.pos[1]-ln[1])
                vec_cable = ((self.pos[0] - ln[0])/cable_len, (self.pos[1] - ln[1])/cable_len)
                dir_cable = math.atan2(vec_cable[1], vec_cable[0])

                # Insert the player's velocity as a virtual outwards movement, so it gets cancelled out in the stuff
                # below.
                vec_movement = (self.dir[0] + vec_cable[0] * p_outward_component,
                        self.dir[1] + vec_cable[1] * p_outward_component)
                
                angle = angle_diff(vec_cable, vec_movement)
                outward_component = outward_speed(vec_cable, vec_movement)

                if outward_component > 0:
                    #print "Outside pull"

                    # remove the outward component of motion
                    # Inject a force such that the velocity along ln drops to 0

                    # Negate any outwards force
                    # rotate around pi radians -> reverse sign
                    self.dir[1] += math.sin(dir_cable + math.pi) * outward_component
                    self.dir[0] += math.cos(dir_cable + math.pi) * outward_component

                    # Outwards force should now be zeroed, fixed pull inwards
                    if self.pull:
                        # Dodgy hack to stop the hook from orbiting - at this point the motion will be entirely
                        # tangential, so damp it a bit
                        self.dir[0] -= self.dir[0] * 10 * ts
                        self.dir[1] -= self.dir[1] * 10 * ts

                        self.dir[0] -= vec_cable[0] * self.pull_force * ts
                        self.dir[1] -= vec_cable[1] * self.pull_force * ts
                else:
                    if self.pull:
                        if -outward_component < self.pull_force:
                            # TODO: cap the acceleration and speed (don't let it overshoot)
                            # Sort of modelling slack here... not good
                            self.dir[0] -= vec_cable[0] * self.pull_force * ts
                            self.dir[1] -= vec_cable[1] * self.pull_force * ts
                        # print "Acceleration pull"
                #self.pull_force += 500*ts
                #print self.pull_force

            # Hook end collision checking
            hook_speed = math.hypot(*self.dir)
            speed_factor = ts
            if hook_speed * ts < 5.0:
                speed_factor = 5.0/hook_speed
            hook_collision = self.world.collide_line(self.pos, (self.pos[0] + self.dir[0]*speed_factor,
                self.pos[1] + self.dir[1]*speed_factor))
            #print "start %s end %s" % (self.pos, (self.pos[0] + self.dir[0] * ts, self.pos[1] + self.dir[1] * ts))
            if hook_collision:
                hook_collision = hook_collision[0] # Don't care about the actual collision point
                # hook_collision is the line, find the angle
                angle = angle_diff(self.dir, (hook_collision[1][0] - hook_collision[0][0],
                    hook_collision[1][1] - hook_collision[0][1]))
                print "Collision: %s %f" % (str(hook_collision), angle * 180 / math.pi)
                # Yay radian conversion
                # and not... hoook? release? pull? some flags...
                # TODO: check impact (inwards) velocity as well as angle?
                if not self.pull and math.fabs(angle) > (30.0*math.pi / 180.0) and math.fabs(angle) < (150*math.pi / 180.0):
                    #print "Stuck!"
                    self.stuck = True
                    self.dir[0] = 0.0
                    self.dir[1] = 0.0
                    self.brake = True
                else:
                    # Angle too shallow, glance off
                    self.dir[0], self.dir[1] = vec_bounce(hook_collision, self.dir)

            #print "After: %f %f" % (self.dir[0], self.dir[1])
            self.pos[0] += self.dir[0] * ts
            self.pos[1] += self.dir[1] * ts

            if distsq(self.origin.pos, self.pos) > 100:
                self.release = False

        # Cable is stuck
        else:
            node = self.first_node()
            cable_len = math.hypot(node[0]-self.origin.pos[0], node[1]-self.origin.pos[1])
            vec_cable = ((node[0] - self.origin.pos[0])/cable_len, (node[1] - self.origin.pos[1])/cable_len)

            if self.pull:
                self.origin.push((vec_cable[0] * 500, vec_cable[1]*500))

            if self.brake:
                # Don't let the cable play out any further (i.e. let the player hang off)
                inward_component = outward_speed(vec_cable, self.origin.velocity)
                if inward_component < 0:
                    dir_pcable = math.atan2(vec_cable[1], vec_cable[0])
                    self.origin.velocity[0] -= math.cos(dir_pcable) * inward_component
                    self.origin.velocity[1] -= math.sin(dir_pcable) * inward_component

        # World collision-y stuff
        # While blocks don't move, only the origin (player) and final (hook) nodes really need to be checked
        last_node = self.origin.pos
        new_nodes = []
        for idx, node in enumerate(self.path()):
            collision = self.world.collide_corner(last_node, node)
            if collision:
                new_nodes.append((idx, collision))
            last_node = node
        for idx, node in new_nodes:
            # TODO: probably have to increment idx to make this work right
            # and sort the insert list
            self.nodes.insert(idx, node)
            self.node_angles.insert(idx, math.pi)

        # Rope unwinding
        # One approach: if two nodes can be joined without a collision, the node can probably be deleted
        # Alternative: monitor the angle of the joint, if it goes beyond 180 unwind

        # Length-based unwinding
        if self.nodes:
            if distsq(self.origin.pos, self.nodes[0]) < 10:
                del self.nodes[0]
        if self.nodes:
            if distsq(self.nodes[-1], self.pos) < 10:
                del self.nodes[-1]

        # For now, only assume the first and last nodes can unwind
        if self.nodes:
            # Player end

            angle_np = math.atan2(self.origin.pos[1] - self.nodes[0][1],self.origin.pos[0] - self.nodes[0][0])
            if len(self.nodes) == 1:
                next_node = self.pos
            else:
                next_node = self.nodes[1]
            # TODO: probably need to fiddle with signs to make this come out right
            angle_nn = math.atan2(next_node[1] - self.nodes[0][1], next_node[0] - self.nodes[0][0])
            #print "np: %f nn: %f" % (angle_np / math.pi * 180.0, angle_nn / math.pi * 180.0)

            #print (angle_nn - angle_np) / math.pi * 180.0
            angle = math.fabs(angle_nn - angle_np)
            if self.angle_hook == 0:
                if angle > math.pi:
                    self.angle_hook = 1
                else:
                    self.angle_hook = -1
            else:
                # TODO: there's probably a cleaner way to express this
                if self.angle_hook == 1 and angle < math.pi or\
                        self.angle_hook == -1 and angle > math.pi:
                            # Unwind!
                            del self.nodes[0]
                            self.angle_hook = 0
            # Hook end
            # Ewwww, duplication
        if self.nodes:  # In case the only node was deleted
            angle_np = math.atan2(self.pos[1] - self.nodes[-1][1],self.pos[0] - self.nodes[-1][0])
            if len(self.nodes) == 1:
                next_node = self.origin.pos
            else:
                next_node = self.nodes[-2]
            # TODO: probably need to fiddle with signs to make this come out right
            angle_nn = math.atan2(next_node[1] - self.nodes[-1][1], next_node[0] - self.nodes[-1][0])

            #print (angle_nn - angle_np) / math.pi * 180.0
            angle = math.fabs(angle_nn - angle_np)
            if self.angle_origin == 0:
                if angle > math.pi:
                    self.angle_origin = 1
                else:
                    self.angle_origin = -1
            else:
                # TODO: there's probably a cleaner way to express this
                if self.angle_origin == 1 and angle < math.pi or\
                        self.angle_origin == -1 and angle > math.pi:
                            # Unwind!
                            del self.nodes[-1]
                            self.angle_origin = 0

        # Update length
        last_node = self.origin.pos
        self.real_length = 0
        for node in self.nodes:
            self.real_length += math.hypot(last_node[0] - node[0], last_node[1] - node[1])
            last_node = node
        self.real_length += math.hypot(self.pos[0] - last_node[0], self.pos[1] - last_node[1])
        if (not self.stuck and not self.pull) or (self.pull and self.real_length < self.slack_length):
            self.slack_length = self.real_length
        if self.real_length > ROPE_LENGTH:
            self.brake = True
        #print "Length: %f Slack: %f" % (self.real_length, self.slack_length)
  
        # TODO: don't idle the cable if the player is hanging off something
        if not self.release and not self.nodes and distsq(self.origin.pos, self.pos) < 64:
            self.dir[0] = self.dir[1] = 0
            self.idle = True
            # Reset/delete/whatever

    def retract(self, value):
        # If hook has been released, don't let player stop the pull
        # If it's not hooked, calling a retract will pull it in all the way
        # Maybe a retract shouldn't call an unhooked line, to allow buffering the pull...
        if not self.stuck:
            return
        if self.stuck or value:
            self.pull = value
        self.release = False
        self.brake = True

    def detach(self):
        self.stuck = False
        self.pull = True

    def path(self):
        for node in self.nodes:
            yield node
        yield self.pos

class Player(object):
    def __init__(self, pos, world):
        self.pos = [pos[0], pos[1]]
        self.velocity = [0.0, 0.0]
        self.f_accum = [0, 0]
        self.on_ground = False
        self.world = world

    def update(self, ts):

        # Gravity
        # Should go through f_accum?
        self.velocity[1] -= GRAVITY * ts

        if self.on_ground:
            # Why does this act weirdly?
            self.velocity[0] -= self.velocity[0] * 100 * ts
        #print self.velocity[0]

        self.velocity[0] += self.f_accum[0] * ts
        self.velocity[1] += self.f_accum[1] * ts
 
        self.f_accum[0] = 0
        self.f_accum[1] = 0

        speed = math.hypot(*self.velocity)
        speed_factor = ts
        # Stretch the collision vector a bit
        if speed > 0:
            if speed*ts < 5.0:
                #print "Speed cap %f" % (speed*ts)
                speed_factor = 5.0/speed
            else:
                #print "Uncapped %f" % (speed*ts)
                pass
            collision = self.world.collide_line(self.pos, (self.pos[0] + self.velocity[0]*speed_factor,
                self.pos[1] + self.velocity[1]*speed_factor))
            if collision:
                #self.velocity[0], self.velocity[1] = vec_bounce(collision, self.velocity)

                # Fix position a little way back from the collision point
                # Hrm, isn't the collision vector guaranteed to be along the speed vector anyway?
                impact = (collision[1][0] - self.pos[0], collision[1][1] - self.pos[1])
                impact_s = math.hypot(impact[0], impact[1])
                # Is there a guarantee this won't be 0? I don't think so...
                impact = (impact[0] / impact_s, impact[1] / impact_s)

                self.pos[0] = collision[1][0] - impact[0] * 2.0
                self.pos[1] = collision[1][1] - impact[1] * 2.0

                # TODO: put on_ground back in here

                self.velocity[0], self.velocity[1] = (0, 0)
                # TODO: record inwards velocity use as sticking time?

        self.pos[0] += self.velocity[0] * ts
        self.pos[1] += self.velocity[1] * ts

        # Collisions

    def push(self, force):
        self.f_accum[0] += force[0]
        self.f_accum[1] += force[1]

class Stage(object):
    def __init__(self):
        # Top left, bottom right
        #self.blocks = [(800, 500, 900, 600),
        #        (700, 300, 800, 400)]
        self.blocks = []
        for x in range(6):
            x = random.randrange(100, 900)
            y = random.randrange(100, 500)
            width = random.randrange(50, 400)
            height = random.randrange(50, 400)
            self.blocks.append((x, y, x+width, y+height))

        y = FIELD_HEIGHT
        while y > 200:
            y -= random.randrange(100, 300)
            for x in range(2):
                x = random.randrange(100, 900)
                self.blocks.append((x, y-100, x+100, y))
        #self.blocks = [(0, 500, 1024, 550)]
        #self.blocks = [(550, 0, 600, 768)]
        self.walls = [-FIELD_WIDTH/2, 10, FIELD_WIDTH/2, FIELD_HEIGHT]

    def collide_point(self, coord):
        if coord[0] < 0 or coord[0] > 1024:
            pass
        if coord[1] < 0 or coord[1] > 768:
            pass
        return False

    def _block_lines(self, block):
        # top
        yield ((block[0], block[1]), (block[2], block[1]))
        # right (t-b)
        yield ((block[2], block[1]), (block[2], block[3]))
        # bottom (l-r)
        yield ((block[0], block[3]), (block[2], block[3]))
        # left (t-b)
        yield ((block[0], block[1]), (block[0], block[3]))

    def _gradient(self, line):
        if line[0][0] == line[1][0]:
            return float('NaN') # vertical
        return (line[1][1] - line[0][1]) / (line[1][0] - line[0][0])

    def _line_collision(self, l1, l2):

        # Performance tweak: check if the bounding boxes for the lines intersect
        m1 = self._gradient(l1)
        m2 = self._gradient(l2)

        # Parallel lines
        if (math.isnan(m1) and math.isnan(m2)):
            if l1[0][0] != l2[0][0]:
                return False
            print "Parallel vertical"
            # TODO: implement intersection check and generate a collision point
            return False

        if math.isnan(m2):
            # Swap around all m1 and m2, to decomplicate stuff
            l1, l2 = l2, l1
            m1, m2 = m2, m1 # Yay python
        if math.isnan(m1):
            # Special case collision for a straight line. They can't both be straight due to the parallel check above
            collision_x = l1[1][0]
            if collision_x < min(l2[0][0], l2[1][0]) or\
                    collision_x > max(l2[0][0], l2[1][0]):
                        return False
            c2 = l2[0][1] - m2 * l2[0][0]
            collision_y = c2 + collision_x * m2
            # No need to check against l2 (the non-vertical line) because ...
            if collision_y < min(l1[0][1], l1[1][1]) or\
                    collision_y > max(l1[0][1], l1[1][1]):
                        return False
            dots.append((collision_x, collision_y))
            return (collision_x, collision_y)

        # Regular collision

        c1 = l1[0][1] - m1 * l1[0][0]
        c2 = l2[0][1] - m2 * l2[0][0]
        if m1 == m2:
            if c1 != c2:
                return False
            print "Aligned!"
            # TODO: check for overlap and choose a collision point
            return False

        if m2 == 0:
            collision_y = l2[0][1]
            collision_x = (collision_y - c1) / m1
        elif m1 == 0:
            collision_y = l1[0][1]
            collision_x = (collision_y - c2) / m2
        else:
            collision_x = (c2 - c1) / (m1 - m2)
            collision_y = collision_x * m1 + c1

        #TODO:  Can this be crunched down to 2 cases by building up the min/max?
        if collision_x >= min(l1[0][0], l1[1][0]) and collision_x <= max(l1[0][0], l1[1][0]) and\
                collision_y >= min(l1[0][1], l1[1][1]) and collision_y <= max(l1[0][1], l1[1][1]) and\
                collision_x >= min(l2[0][0], l2[1][0]) and collision_x <= max(l2[0][0], l2[1][0]) and\
                collision_y >= min(l2[0][1], l2[1][1]) and collision_y <= max(l2[0][1], l2[1][1]):

                    # DEBUG
                    dots.append((collision_x, collision_y))
                    return (collision_x, collision_y)
        return False

    # ASSUMPTIONS: blocks form closed shapes
    # Returns the corner where the collision should have occurred (assuming they can't happen on flat sections...)
    def collide_corner(self, start, end):
        global dots
        # If line collides with a piece of geometry, find the point where it should have wrapped (a corner)
        for block in self.blocks:
            # Optimisation: bounding boxes
            # Optimisation: pre-calculate mx and c for the input line (and everything else...)
            collided = []
            for line in self._block_lines(block):
                if self._line_collision((start, end), line):
                    # Interesting point will be at the intersection of two hit lines
                    # if it's top and bottom of the block... physics assumptions broken
                    collided.extend([line[0], line[1]])
                    #return True

            # Interesting point should have two entries in the list
            #print collided
            for vertex in collided:
                if collided.count(vertex) > 1:
                    dots.append(vertex)
                    # TODO: make sure this pushes outside a bit to prevent repeat collisions
                    if vertex[0] == block[0]:
                        xd = -0.1
                    else:
                        xd = 0.1
                    if vertex[1] == block[1]:
                        yd = -0.1
                    else:
                        yd = 0.1
                    return (vertex[0] + xd, vertex[1] + yd)
        return False
    # This may lead to weirdness if the player clips a corner and hits two lines at once...
    def collide_line(self, start, end):
        # Wall collisions - left separate because they'll probably be dealt with differently later
        for line in self._block_lines(self.walls):
            point = self._line_collision((start, end), line)
            if point:
                return (line, point)

        for block in self.blocks:
            for line in self._block_lines(block):
                point = self._line_collision((start, end), line)
                if point:
                    return (line, point)
        return False

    # Returns the block that caused the collision
    def collide_point(self, point):
        for block in self.blocks:
            if point[0] > block[0] and point[0] < block[2] and\
                    point[1] > block[1] and point[1] < block[3]:
                        return block
        return False

####################### Maths helpers
def nvec(src, dst):
    dx = dst[0] - src[0]
    dy = dst[1] - src[1]
    fac = 1/math.sqrt(dx*dx+dy*dy)
    return (dx*fac, dy*fac)

def distsq(src, dst):
    dx = dst[0] - src[0]
    dy = dst[1] - src[1]
    return (dx*dx + dy*dy)

def angle_diff(a1, a2):
    dir_a1 = math.atan2(a1[1], a1[0])
    dir_a2 = math.atan2(a2[1], a2[0])

    # Can't just fmod to 180 here - -190 ends up at -10 instead of 170
    # Difference between the angles, shift to 0 <= x < 2pi
    # angle_diff = math.fmod(dir_cable - dir_motion + math.pi*2, math.pi*2)
    # Interested in the absolute difference. Wrap it around to make 340 into -20
    # i.e. shift range do -pi <= x < pi

    angle = math.fmod(dir_a1 - dir_a2 + math.pi*2, math.pi*2)
    if angle > math.pi:
        angle -= math.pi*2

    return angle

# TODO: incomplete bounce, take away a bit of velocity in the process
def vec_bounce(axis, point):
    # http://en.wikipedia.org/wiki/Coordinate_rotations_and_reflections seems to have the appropriate
    # matrix...
    # [cos2t  sin2t][x]
    # [sin2t -cos2t][y]

    angle = math.atan2(axis[1][1] - axis[0][1], axis[1][0] - axis[0][0])
    c2t = math.cos(2*angle)
    s2t = math.sin(2*angle)
    nx = c2t * point[0] + s2t * point[1]
    ny = s2t * point[0] - c2t * point[1]
    return (nx, ny)

# Scalar speed away from src.
# Actually just a projection/dot product
# src is the vector, dst is to be projected
# TODO: rewrite this as a.^b? (shorter scalar projection definition)
def outward_speed(src, dst):
    len = math.hypot(dst[1] - src[1], dst[0] - src[0])
    angle = angle_diff(dst, src)
    return math.cos(angle) * math.hypot(*dst)

###################### Misc game code

screen = pygame.display.set_mode((1024, 768), pygame.DOUBLEBUF)

counter = pygame.time.Clock()

world = Stage()

player = Player((512, FIELD_HEIGHT-5), world)
dir_x = 0
dir_y = 0

# Coordinate of top-left corner
viewport_x = 0
viewport_y = 0

hook = Hook(player, world)

path = []
pause = False

while 1:
    evt = pygame.event.poll()
    while evt.type != pygame.NOEVENT:
        if evt.type == pygame.QUIT:
            sys.exit()
        elif evt.type == pygame.KEYDOWN:
            if evt.key == pygame.K_LEFT:
                dir_x = -1
            elif evt.key == pygame.K_RIGHT:
                dir_x = 1
            elif evt.key == pygame.K_UP:
                dir_y = -1
            elif evt.key == pygame.K_DOWN:
                dir_y = 1
            elif evt.key == pygame.K_SPACE:
                pause = not pause
        elif evt.type == pygame.KEYUP:
            if evt.key == pygame.K_LEFT or evt.key == pygame.K_RIGHT:
                dir_x = 0
            elif evt.key == pygame.K_UP or evt.key == pygame.K_DOWN:
                dir_y = 0
        elif evt.type == pygame.MOUSEBUTTONDOWN:
            if evt.button == 1:
                if hook.idle:
                    target = nvec(player.pos, (evt.pos[0] + viewport_x, evt.pos[1] + viewport_y))
                    target = (target[0]*900, target[1]*900)
                    hook.fire(target)
                else:
                    hook.detach()

            elif evt.button == 3:
                hook.retract(True)
        elif evt.type == pygame.MOUSEBUTTONUP:
             if evt.button == 3:
                 hook.retract(False)
        evt = pygame.event.poll()

    ts = counter.tick() / 1000.0
    # TODO: cap ts to ensure physics don't break

    #player.push((dir_x*500, dir_y*500))
    player.push((dir_x*1000, dir_y*1000))

    if not pause:
        hook.update(ts)
        player.update(ts)

    # Working on the assumption the field is 2000 high, for now
    # Chase? swing ahead? 
    # Lock player in center for now
    #if player.pos[1] > 200 and player.pos[1] - viewport_y > 500:
        viewport_y = player.pos[1] - 350
        viewport_x = player.pos[0] - 512

    screen.fill(bg)

    pygame.draw.circle(screen, fg, (int(player.pos[0] - viewport_x), int(player.pos[1] - viewport_y)), 5)

    # Outside borders
    pygame.draw.rect(screen, black, (0, FIELD_HEIGHT - viewport_y, 1025, 1000))
    pygame.draw.rect(screen, black, (0, -1000 - viewport_y, 1025, 1000))

    for block in world.blocks:
        pygame.draw.rect(screen, fg, (block[0] - viewport_x, block[1] - viewport_y, block[2]-block[0], block[3]-block[1]))

    if not hook.idle:
        last_node = (player.pos[0] - viewport_x, player.pos[1] - viewport_y)
        for node in hook.path():
            this_node = (int(node[0] - viewport_x), int(node[1] - viewport_y))
            pygame.draw.line(screen, rg, last_node, this_node)
            last_node = this_node

    for dot in dots:
        pygame.draw.circle(screen, mg, (int(dot[0] - viewport_x), int(dot[1] - viewport_y)), 3)
    dots = []
    pygame.display.flip()

