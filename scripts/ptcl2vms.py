#!/usr/bin/python
#imports
import string
import sys
import os
import shutil
import copy
import math
#the usage string, printed when the user is abusing our tool, lol
usage =  """usage: ptcl2vms.py [options] <infile1> [[options] <infile2>] ... <outfile>
valid options are...
    -stride (int) adjusts with what stride we read and generate points
    -radius (float) adjusts the diameter of the generated point meshes
    -ref (int) adjusts tesselation level of point meshes
    -startts (int) sets the starting timestep of the output
"""
#TODO: try it with more than one timestep
#TODO: fix stride problem
#a bunch of vector manipulation functions! :D
def dot(a, b):
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2])
def mag2(v):
    return dot(v, v)
def mag(v):
    return math.sqrt(mag2(v))
def div(v, n):
    return v[0] / n, v[1] / n, v[2] / n
def mul(v, n):
    return v[0] * n, v[1] * n, v[2] * n
def add(a, b):
    return a[0] + b[0], a[1] + b[1], a[2] + b[2]
def sub(a, b):
    return a[0] - b[0], a[1] - b[1], a[2] - b[2]
def norm(a):
    return div(a, mag(a))
def resize(a, n):
    return mul(a, n / mag(a))
def centroid(points):
    c = (0.0, 0.0, 0.0)
    for p in points:
        c = add(c, p)
    c = div(c, len(points))
    return c
#A tetrahedron for subdivision by subdivnorm
overts = [ \
    norm((-1, -1,  1)),
    norm(( 1,  1,  1)),
    norm((-1,  1, -1)),
    norm(( 1, -1, -1))]
ofaces = [ \
    (0, 1, 2),
    (3, 2, 1),
    (0, 3, 1),
    (3, 0, 2)]
def subdivnorm(verts, faces):
    nfaces = len(faces)
    for faceIndex in range(0, nfaces) :
        face = faces[faceIndex] #choose a face to subdivide
        fverts = [] #get the vertices of the face
        for i in face:
            fverts.append(verts[i])
        #ctr = norm(centroid(fverts)) #get the centroid of the face
        everts = [] #get and normalize the new edge vertices
        for i in range(0, len(fverts)):
            ni = (i + 1) % len(fverts)
            everts.append(norm(centroid([fverts[i], fverts[ni]])))
        #add all the new vertices, remembering where we stored them
        eidx = len(verts)
        verts.extend(everts)
        #replace the existing face with the first new face
        faces[faceIndex] = (eidx, eidx + 1, eidx + 2)
        #build new faces, appending them to the faces array
        for i in range(0, len(face)):
            ni = (i + 1) % len(face)
            faces.append((eidx + i, face[ni], eidx + ni))
def catmullnorm(verts, faces) :
    nfaces = len(faces)
    for faceIndex in xrange(nfaces) :
        face = faces[faceIndex] #choose a face to subdivide
        fverts = [] #get the vertices of the face
        for i in face:
            fverts.append(verts[i])
        ctr = norm(centroid(fverts)) #get the centroid of the face
        everts = [] #get and normalize the new edge vertices
        for i in range(0, len(fverts)):
            ni = (i + 1) % len(fverts)
            everts.append(norm(centroid([fverts[i], fverts[ni]])))
        #add all the new vertices, remembering where we stored them
        cidx = len(verts)
        verts.append(ctr)
        eidx = len(verts)
        verts.extend(everts)
        #rebuild the existing face as the first new face
        faces[faceIndex] = (eidx, face[1], eidx + 1, cidx)
        #build new quads, appending them to the faces array
        for i in range(1, len(face)):
            ni = (i + 1) % len(face)
            faces.append((eidx + i, face[ni], eidx + ni, cidx))
def showMesh(verts, faces):
    for v in verts:
        print("v " + str(v[0]) + " " + str(v[1]) + " " + str(v[2]))
    for f in faces:
        if len(f) == 3:
            print("f "+str(f[0]+1)+" "+str(f[1]+1)+" "+str(f[2]+1))
        else:
            print("f "+str(f[0]+1)+" "+str(f[1]+1)+" "+str(f[2]+1)+" "+str(f[3]+1))
    print("")
#remove the scriptname from argv, but store it just in case :P
scriptname = sys.argv.pop(0)

#check for valid number of arguments
if len(sys.argv) < 2 :
    print(">>>ERROR: not enough arguments!\n" + usage)
    exit(-1)
#get the name of output file and directory
vmsname = sys.argv.pop()
print(">>>OUTFILE: " + vmsname)
dirname = vmsname[0:vmsname.rfind('.')] + "_data"
#remove any preexisting data by the same name
if os.path.exists(dirname) :
    shutil.rmtree(dirname, ignore_errors=False, onerror=None)
if os.path.exists(vmsname) :
    os.remove(vmsname)
#make data directory
os.mkdir(dirname)
#process each option or file
stride = 1
ref = 0
radius = 1
startts = 0
plylist = []
while len(sys.argv) > 0 :
    #get the next argument
    arg = sys.argv.pop(0)
    #handle options
    if arg[:1] == "-" :
        if arg == "-stride" :
            stride = int(sys.argv.pop(0))
            print("stride = " + str(stride))
        elif arg == "-radius" :
            radius = float(sys.argv.pop(0))
            print("radius = " + str(radius))
        elif arg == "-ref" :
            ref = int(sys.argv.pop(0))
            print("refinement = " + str(ref))
        elif arg == "-startts" :
            startts = int(sys.argv.pop(0))
            print("startts = " + str(startts))
        else:
            print(">>>ERROR: unknown option: '" + arg + "'\n" + usage)
            exit(-1)
        continue
    #open input and output files
    print(">>>READING: " + arg)
    infile = open(arg)
    #get input from input file
    counter = 1
    points = []
    for line in infile.readlines() :
        strc = line.split()
        if len(strc) < 3:
            continue
        coords = []
        for i in strc:
            coords.append(float(i))
        counter += 1
        if counter >= stride :
            points.append(coords)
            counter = 1
    infile.close()
    #generate a sphere with specified refinement, starting with our tetrahedron
    lverts = copy.deepcopy(overts)
    lfaces = copy.deepcopy(ofaces)
    for i in range(ref) :
        subdivnorm(lverts, lfaces)
    lnorms = copy.deepcopy(lverts)
    for i in range(0, len(lverts)) :
        lverts[i] = resize(lverts[i], radius)
    #copy the sphere to various positions! :D
    verts = []
    faces = []
    norms = []
    offset = 0
    #this loop runs once for each copy (each must be at a different position)
    for pos in range(0, len(points), stride) :
        #add the current position to each point before appending it to verts
        for vert in lverts :
            verts.append(add(vert, points[pos]))
        for nm in lnorms :
            norms.append(nm)
        for face in lfaces :
            faces.append((face[0] + offset,
                          face[1] + offset,
                          face[2] + offset))
        #the next face will index its verts from the next starting position
        offset = len(verts)
    #write the model to a file!
    plyname = dirname+"/"+os.path.basename(arg[0:arg.rfind(".")])+".ply"
    print(">>>WRITING: " + plyname)
    ply = open(plyname, "w")
    ply.write('ply\nformat ascii 1.0\nelement vertex ' + str(len(verts)) + '\nproperty float x\nproperty float y\nproperty float z\nproperty float nx\nproperty float ny\nproperty float nz\nelement face ' +  str(len(faces)) + '\nproperty list uchar int vertex_indices\nend_header\n')
    for vi in range(len(verts)):
        v = verts[vi]
        n = norms[vi]
        ply.write(str(v[0])+" "+str(v[1])+" "+str(v[2])+" "\
                 +str(n[0])+" "+str(n[1])+" "+str(n[2])+"\n")
    for f in faces:
        ply.write("3 "+str(f[0])+" "+str(f[1])+" "+str(f[2])+"\n")
    ply.close()
    plylist.append(plyname)
#write the VMS file, listing all our outputs in it!
if len(plylist) < 1:
    print("ERROR: No input files specified,\n       or one input and no outputs!\n")
    exit(1)
vms = open(vmsname, "w")
counter = 0
vms.write('<?xml version="1.0" encoding="ISO-8859-1" standalone="yes"?>\n<ModelScene>\n')
for i in range(startts) :
    vms.write('<!-- ts ' + str(counter) + ' -->\n')
    vms.write('<TimeStep>\n')
    vms.write('</TimeStep>\n')
    counter += 1
for ply in plylist :
    vms.write('<!-- ts ' + str(counter) + ' -->\n')
    vms.write('<TimeStep>\n')
    vms.write('<File>' + ply + '</File>\n')
    vms.write('</TimeStep>\n')
    counter += 1
vms.write('</ModelScene>\n')
vms.close()

