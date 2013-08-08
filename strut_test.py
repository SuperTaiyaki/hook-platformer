#!/usr/bin/python2

# vim: ts=4 sw=4 et

import pygame
import sys
import math

pygame.init()

bg = pygame.Color(0x80808000)
fg = pygame.Color(0xFFFFFFFF)
mg = pygame.Color(0xff0000ff)

class Node(object):
    def __init__(self, stiffness, mass):
        self.deflection = 0.0
        self.velocity = 0.0
        self.f_accum = 0.0
        self.stiffness = stiffness
        self.mass = mass

        self.damping = 0.8 # Should this be related to stiffness?
        self.spring = stiffness

    def update(self, ts):

        # Account for stored energy in deflection
        self.f_accum -= self.deflection * self.spring # Or something.... should spring be related to stiffness?
        # Actually, should probably be related to spring value - both return and acceleration

        # Apply damping... should this be time-scaled?
        self.f_accum -= self.velocity * self.damping

        # Use f_accum to update velocity
        self.velocity += self.f_accum * self.spring * ts

        self.f_accum = 0

        self.deflection += self.velocity/self.spring * ts

    # Force on endpoint only!
    def force(self, f):
        self.f_accum += f # divide by stiffness / mass?

class Beam(object):
    def __init__(self, stiffnesses, masses, lengths):
        self.nodes = [(Node(s, m), l) for s, m, l in zip(stiffnesses, masses, lengths)]
        self.total_length = sum(lengths)
        return
    def update(self, ts):
        for node, _ in self.nodes:
            node.update(ts)
        return

    def force(self, f, position):
        assert(position >= 0)
        assert(position <= self.total_length)

        nodes = 0
        a_p = 0
        total_stiffness = 0
        while a_p < position:
            a_p += self.nodes[nodes][1]
            total_stiffness += self.nodes[nodes][0].stiffness # Divided by mass?
            nodes += 1

        for node, _ in self.nodes[0:nodes]:
            #node.force(f*node.stiffness/total_stiffness)
            node.force(f/nodes)

        return

screen = pygame.display.set_mode((1024, 768), pygame.DOUBLEBUF)


#strut = Beam([1.0, 1.0, 1.0], [1.0, 1.0, 1.0], [10, 10, 10])
strut1 = Beam([1.0]*5, [1.0]*5,[10.0]*5)
strut2 = Beam([6.0, 4.0, 2.0, 1.5, 1.0], [1.0]*5,[10.0]*5)
#points = [(float(x)*100.0, 500.0) for x in range(10)]
counter = pygame.time.Clock()
#point = Node(1.0, 1.0)

pos_x = 0.0
dir = 0.0
push = False
while 1:
    evt = pygame.event.poll()
    if evt.type == pygame.QUIT:
        sys.exit()
    elif evt.type == pygame.KEYDOWN:
        if evt.key == pygame.K_LEFT:
            dir = -1
        elif evt.key == pygame.K_RIGHT:
            dir = 1
        elif evt.key == pygame.K_SPACE:
            #strut.force(-100, pos_x)
            push = True
    elif evt.type == pygame.KEYUP:
        if evt.key == pygame.K_LEFT or evt.key == pygame.K_RIGHT:
            dir = 0
        elif evt.key == pygame.K_SPACE:
            push = False


    #point.force(-100.0)
    #strut.force(-100.0, 30.0)
    #strut.force(-400.0, 35.0)

    ts = counter.tick() / 1000.0

    if dir != 0:
        pos_x += dir * 50 * ts
    if push:
        strut1.force(0.2, pos_x)
        strut2.force(0.2, pos_x)

    #point.update(ts)
    strut1.update(ts)
    strut2.update(ts)

    screen.fill(bg)
 
    #pygame.draw.lines(screen, fg, False, points)
    #pygame.draw.line(screen, fg, (100, 500), (500, 500 + point.deflection * 100.0))

    pygame.draw.circle(screen, mg, (int(pos_x*10 + 100), 500), 5)

    def render(strut, y):
        heading = 0
        x = 100
        for point, length in strut.nodes:
            pygame.draw.circle(screen, fg, (int(x), int(y)), 10)

            nh = math.atan2(point.deflection*10, 1)
            heading += nh
            nx = x + 100*math.cos(heading)
            ny = y + 100*math.sin(heading)
            pygame.draw.line(screen, fg, (x, y), (nx, ny))

            x = nx
            y = ny
    render(strut1, 200)
    render(strut2, 400)

    pygame.display.flip()
