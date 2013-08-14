#!/usr/bin/env python2

# Take a SVG file and convert it to a stage
# Only rects are supported
# Place a text node with the value "start" to indicate the player start point
# Only tested with inkscape 0.48

import sys
from xml.dom.minidom import parse

if len(sys.argv) < 3:
	print("Usage: %s [svg in] [stage out]")
	sys.exit(0)

f = open(sys.argv[2], "w")
doc = parse(sys.argv[1])

# svg -> g -> rect, text
svg = doc.getElementsByTagName("svg")
if svg.length != 1:
	print("Too many SVG components; exiting.")
	sys.exit(0)
svg = svg.item(0)

# Y0 is at the top, find the max value to flip all coords
# This is a horribly wrong way to kill the 'px'...
height = float(svg.attributes['height'].value.strip('px'))

rects = svg.getElementsByTagName("rect")
for rect in rects:
	x1 = float(rect.attributes['x'].value)
	y1 = height - float(rect.attributes['y'].value)
	x2 = x1 + float(rect.attributes['width'].value)
	y2 = y1 - float(rect.attributes['height'].value)

	coords = (min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2))

	f.write("b %f %f %f %f x\n" % coords)
points = svg.getElementsByTagName("text")
for point in points:
	x = float(point.attributes['x'].value)
	y = height - float(point.attributes['y'].value)
	text = point.getElementsByTagName("tspan")
	if text.length != 1:
		continue
	value = text.item(0).firstChild.data
	if value.lower() == "start":
		f.write("p %f %f START\n" % (x, y))

