#!/usr/bin/python2

# vim: ts=4 sw=4 et

import pygame
import sys
import math

pygame.init()

bg = pygame.Color(0x80808000)
fg = pygame.Color(0xFFFFFFFF)
mg = pygame.Color(0xff0000ff)
color2 = pygame.Color(0x00ff00ff)
GRAVITY=-10.0
STRETCH_FACTOR=200.0

dots = []

class Hook(object):
    def __init__(self, pos, world):
        self.origin = pos
        self.length = 0
        self.pull = False
        self.stuck = False
        self.release = False
        self.nodes = []
        self.idle = True
        self.world = world

    def fire(self, dir):
        self.idle = False
        self.dir = [dir[0], dir[1]]
        self.pos = [self.origin.pos[0], self.origin.pos[1]]
        self.nodes = []
        self.node_angles = []
        self.release = True
        self.pull = False
        self.stuck = False
        self.angle_hook = 0
        self.angle_origin = 0
        self.pull_force = 4000
        self.slack_length = 0
        self.real_length = 0 # tension factor

    def detach(self):
        self.stuck = False

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
            if self.pull:
                """
                ### Pure tension-based system - oscillates
                if self.real_length > self.slack_length:
                    stretch_force = self.real_length - self.slack_length * STRETCH_FACTOR
                    ln = self.last_node()
                    cable_len = math.hypot(self.pos[0]-ln[0], self.pos[1]-ln[1])
                    vec_cable = ((self.pos[0] - ln[0])/cable_len, (self.pos[1] - ln[1])/cable_len)
                    self.dir[0] += stretch_force * vec_cable[0] * ts
                    self.dir[1] += stretch_force * vec_cable[1] * ts
                """

                # TODO: IMPLEMENT THIS BIT
# 			Calculate rate at which player is moving away from node
#			subtract speed from hook velocity, so the correction algorithm fixes it up automatically

                # Calculate the player's outwards velocity first

                fn = self.first_node()
                cable_len = math.hypot(self.origin.pos[0] - fn[0], self.origin.pos[1] - fn[1])
                vec_player = (self.origin.pos[0] - fn[0], self.origin.pos[1] - fn[1]) # unscaled
                angle = angle_diff(self.origin.velocity, (vec_player[0], vec_player[1]))
                p_outward_component = 0
                if math.fabs(angle) < math.pi/2:
                    p_outward_component = math.cos(angle) * math.hypot(*self.origin.velocity)
                #print "Player pull: %f" % p_outward_component

                ### Dodgy system that kills outwards force
                # Lock the cable - don't let it grow outwards
                ln = self.last_node()
                cable_len = math.hypot(self.pos[0]-ln[0], self.pos[1]-ln[1])
                vec_cable = ((self.pos[0] - ln[0])/cable_len, (self.pos[1] - ln[1])/cable_len)
                dir_cable = math.atan2(vec_cable[1], vec_cable[0])

                # Insert the player's velocity as a virtual outwards movement, so it gets cancelled out in the stuff
                # below
                vec_movement = (self.dir[0] + vec_cable[0] * p_outward_component,
                        self.dir[1] + vec_cable[1] * p_outward_component)
                
                angle = angle_diff(vec_cable, vec_movement)

                if math.fabs(angle) < math.pi/2:
                    print "Outside pull"

                    # remove the outward component of motion
                    # Inject a force such that the velocity along ln drops to 0
                    #angle_diff = dir_motion - dir_cable
                    # Scale factor
                    outward_component = math.cos(angle) # Magnitude of the force heading outwards
                    outward_component *= math.hypot(*vec_movement)

                    # Negate any outwards force
                    # rotate around pi radians -> reverse sign (for cos, anyway)
                    self.dir[1] += math.sin(dir_cable + math.pi) * outward_component
                    self.dir[0] += math.cos(dir_cable + math.pi) * outward_component

                    # Speed damping that doesn't work right
                    #self.dir[0] -= self.dir[0] * 0.5 * ts
                    #self.dir[1] -= self.dir[1] * 0.5 * ts

                    # Outwards force should now be zeroed, fixed pull inwards
                    #self.dir[0] -= vec_cable[0] * self.pull_force * ts
                    #self.dir[1] -= vec_cable[1] * self.pull_force * ts

                    # TODO: factor in player velocity 
                else:
                    inward_component = -math.cos(angle) * math.hypot(self.dir[0], self.dir[1])
                    if inward_component < self.pull_force:
                        # TODO: cap the acceleration and speed (don't let it overshoot)
                        # Sort of modelling slack here... not good
                        #self.dir[0] -= vec_cable[0] * self.pull_force * ts
                        #self.dir[1] -= vec_cable[1] * self.pull_force * ts
                        pass
                    print "Acceleration pull"
                #self.pull_force += 500*ts
                #print self.pull_force


            #print "After: %f %f" % (self.dir[0], self.dir[1])
            self.pos[0] += self.dir[0] * ts
            self.pos[1] += self.dir[1] * ts

            if distsq(self.origin.pos, self.pos) > 100:
                self.release = False

            # TODO: replace with proper collision check
            if self.pos[0] < 1 or self.pos[0] > 1024 or self.pos[1] < 0 or self.pos[1] > 768:
                # Bring it back slightly
                if self.pos[0] < 0:
                    self.pos[0] = 1
                elif self.pos[0] > 1024:
                    self.pos[0] = 1023
                if self.pos[1] < 0:
                    self.pos[1] = 1
                elif self.pos[1] > 768:
                    self.pos[1] = 767
                self.stuck = True
                self.dir[0] = 0
                self.dir[1] = 0

        # Cable is stuck, 
        else:
            if self.pull:
                node = self.first_node()
                cable_len = math.hypot(node[0]-self.origin.pos[0], node[1]-self.origin.pos[1])
                vec_cable = ((node[0] - self.origin.pos[0])/cable_len, (node[1] - self.origin.pos[1])/cable_len)
                self.origin.push((vec_cable[0] * 100, vec_cable[1]*100))

        # World collision-y stuff
        # While blocks don't move, only the origin (player) and final (hook) nodes really need to be checked
        last_node = self.origin.pos
        new_nodes = []
        for idx, node in enumerate(self.path()):
            collision = self.world.collide_line(last_node, node)
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

            print (angle_nn - angle_np) / math.pi * 180.0
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
        print "Length: %f Slack: %f" % (self.real_length, self.slack_length)
  
        # TODO: don't idle the cable if the player is hanging off something
        if not self.release and not self.nodes and distsq(self.origin.pos, self.pos) < 25:
            self.dir[0] = self.dir[1] = 0
            self.idle = True
            # Reset/delete/whatever

    def retract(self):
        self.pull = True
        #self.release = True # DEBUG REMOVE LATER
        self.release = False

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
            self.velocity[0] *= 0.5 * ts
        #print self.velocity[0]

        self.velocity[0] += self.f_accum[0] * ts
        self.velocity[1] += self.f_accum[1] * ts

        
        self.f_accum[0] = 0
        self.f_accum[1] = 0

        self.pos[0] += self.velocity[0] * ts
        self.pos[1] += self.velocity[1] * ts

        # Collisions
        self.on_ground = False
        if self.pos[0] < 10:
            self.pos[0] = 11
        elif self.pos[0] > 1024:
            self.pos[0] = 1023
        if self.pos[1] < 0:
            self.pos[1] = 1
        elif self.pos[1] > 760:
            self.pos[1] = 759

            self.velocity[1] = 0
            self.on_ground = True

    def push(self, force):
        self.f_accum[0] += force[0]
        self.f_accum[1] += force[1]

    def position(self):
        return self.pos

class Stage(object):
    def __init__(self):
        # Top left, bottom right
        self.blocks = [(800, 500, 900, 600),
                (700, 300, 800, 400)]

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
        if m1 == m2 or (math.isnan(m1) and math.isnan(m2)):
            return False

        if math.isnan(m2):
            # Swap around all m1 and m2, to decomplicate stuff
            l1, l2 = l2, l1
            m1, m2 = m2, m1 # Yay python
        if math.isnan(m1):
            # Special case collision for a straight line. They can't both be straight due to the parallel check above
            collision_x = l1[1][0]
            c2 =  l2[0][1] - m2 * l2[0][0]
            collision_y = c2 + collision_x * m2
            if collision_y < min(l1[0][1], l1[1][1]) or\
                    collision_y > max(l1[0][1], l1[1][1]):
                        return False
            return (collision_x, collision_y)

        c1 = l1[0][1] - m1 * l1[0][0]
        c2 = l2[0][1] - m2 * l2[0][0]

        collision_x = (c2 - c1) / (m1 - m2)
        collision_y = collision_x * m1 + c1
        if collision_x >= min(l1[0][0], l1[1][0]) and collision_x <= max(l1[0][0], l1[1][0]) and\
                collision_y >= min(l1[0][1], l1[1][1]) and collision_y <= max(l1[0][1], l1[1][1]) and\
                collision_x >= min(l2[0][0], l2[1][0]) and collision_x <= max(l2[0][0], l2[1][0]) and\
                collision_y >= min(l2[0][1], l2[1][1]) and collision_y <= max(l2[0][1], l2[1][1]):

                    return (collision_x, collision_y)
        return False

    # ASSUMPTIONS: blocks form closed shapes
    # Returns the corner where the collision should have occurred (assuming they can't happen on flat sections...)
    def collide_line(self, start, end):
        global dots
        # If line collides with a piece of geometry, find the point where it should have wrapped (a corner)
        for block in self.blocks:
            collided = []
            for line in self._block_lines(block):
                if self._line_collision((start, end), (line)):
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

    # Returns the block that caused the collision
    def collide_point(self, point):
        for block in self.blocks:
            if point[0] > block[0] and point[0] < block[2] and\
                    point[1] > block[1] and point[1] < block[3]:
                        return block
        return False
screen = pygame.display.set_mode((1024, 768), pygame.DOUBLEBUF)

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

counter = pygame.time.Clock()

world = Stage()

player = Player((512, 700), world)
dir_x = 0
dir_y = 0

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
                    target = nvec(player.pos, evt.pos)
                    target = (target[0]*400, target[1]*400)
                    hook.fire(target)
                else:
                    hook.detach()

            elif evt.button == 3:
                hook.retract()

        evt = pygame.event.poll()

    ts = counter.tick() / 1000.0
    # TODO: cap ts to ensure physics don't break

    player.push((dir_x*500, dir_y*500))

    if not pause:
        hook.update(ts)
        player.update(ts)

    screen.fill(bg)

    pygame.draw.circle(screen, fg, (int(player.pos[0]), int(player.pos[1])), 5)


    for block in world.blocks:
        pygame.draw.rect(screen, fg, (block[0], block[1], block[2]-block[0], block[3]-block[1]))

    if not hook.idle:
        last_node = player.pos
        for node in hook.path():
            this_node = (int(node[0]), int(node[1]))
            pygame.draw.line(screen, fg, last_node, this_node)
            last_node = this_node

    for dot in dots:
        pygame.draw.circle(screen, mg, (int(dot[0]), int(dot[1])), 3)
    dots = []
    pygame.display.flip()

