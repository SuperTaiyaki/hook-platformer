#!/usr/bin/python2

# vim: ts=4 sw=4 et

import pygame
import sys

pygame.init()

bg = pygame.Color(0x80808000)
fg = pygame.Color(0xFFFFFFFF)

class Node:
    def __init__(self, stiffness, mass):
        self.deflection = 0.0
        self.velocity = 0.0
        self.f_accum = 0.0
        self.stiffness = stiffness
        self.mass = mass

        self.damping = 2.0 # correct?
        self.spring = 20.0

    def update(self, ts):

        # Account for stored energy in deflection
        self.f_accum -= self.deflection * self.spring # Or something.... should spring be related to stiffness?
        # Actually, should probably be related to spring value - both return and acceleration

        # Apply damping
        self.f_accum -= self.velocity * self.stiffness * self.damping

        # Use f_accum to update velocity
        self.velocity += self.f_accum * ts

        self.f_accum = 0

        self.deflection += self.velocity * ts

    # Force on endpoint only!
    def force(self, f):
        self.f_accum += f # divide by stiffness / mass?

class Beam:
    # Data sructure: deflection at each node
    def __init__(self, nodes, stiffness):
        self.stiffness = stiffness
        self.nodes = [0.0 for _ in range(nodes)]
        return
    def update(self, ts):
        

        return


screen = pygame.display.set_mode((1024, 768), pygame.DOUBLEBUF)
strut = Beam(10, 1.0)
points = [(float(x)*100.0, 500.0) for x in range(10)]
counter = pygame.time.Clock()
point = Node(1.0, 1.0)

while 1:
    evt = pygame.event.poll()
    if evt.type == pygame.QUIT:
        sys.exit()
    elif evt.type == pygame.KEYDOWN:
        point.force(-100.0)

    ts = counter.tick() / 1000.0
    point.update(ts)

    screen.fill(bg)
 
    #pygame.draw.lines(screen, fg, False, points)
    pygame.draw.line(screen, fg, (100, 500), (500, 500 + point.deflection * 100.0))

    pygame.display.flip()
