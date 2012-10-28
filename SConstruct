import os.path

sources = ['main.cpp', 'marker.cpp', 'page.cpp']
target = 'build/extpage'
objs = []

for s in sources:
    objs.append(s.replace('.cpp', '.o'))

env = Environment(ENV = {'PATH': os.environ['PATH']})

env.AppendUnique(CCFLAGS = ['-O2'])
env.AppendUnique(CCFLAGS = ['-Wall'])

env.AppendUnique(CCFLAGS = ['-I/opt/local/include/'])
env.AppendUnique(CCFLAGS = ['-I/opt/local/include/opencv'])
env.AppendUnique(CCFLAGS = ['-I/opt/local/include/opencv2'])

env.AppendUnique(LINKFLAGS = ['-L/opt/local/lib'])
env.AppendUnique(LINKFLAGS = ['-lopencv_core'])
env.AppendUnique(LINKFLAGS = ['-lopencv_highgui'])
env.AppendUnique(LINKFLAGS = ['-lopencv_imgproc'])
env.AppendUnique(LINKFLAGS = ['-lopencv_calib3d'])

env.Object(source = sources)
env.Program(target = target, source = objs)
