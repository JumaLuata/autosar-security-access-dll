import os
import platform

# Force using MinGW toolchain and inherit system environment
env = Environment(tools=['mingw'], ENV=os.environ)

# Setup 32-bit and 64-bit environments
env_32 = env.Clone()
env_64 = env.Clone()

# Common build options
common_defines = ['BUILD_DLL', 'SECACC_ENABLE_AES128']
common_include = ['include']
sources = ['src/SecAcc_Implementation.c', 'src/AES128.c']

# 32-bit configuration
env_32.Append(CCFLAGS=['-m32', '-Wall', '-O2', '-std=c11'] + ['-D%s' % d for d in common_defines])
env_32.Append(LINKFLAGS=['-m32', '-shared', '-static-libgcc'])
env_32.Append(CPPPATH=common_include)
env_32['TARGET_ARCH'] = 'x86'
env_32['SHLIBPREFIX'] = ''
env_32['SHLIBSUFFIX'] = '.dll'
env_32['OBJPREFIX'] = '../build/obj32/'
env_32['OBJSUFFIX'] = '.o'
env_32['PROGSUFFIX'] = '.dll'
env_32['CC'] = 'gcc'
env_32['CXX'] = 'g++'

# 64-bit configuration
env_64.Append(CCFLAGS=['-m64', '-Wall', '-O2', '-std=c11'] + ['-D%s' % d for d in common_defines])
env_64.Append(LINKFLAGS=['-m64', '-shared', '-static-libgcc'])
env_64.Append(CPPPATH=common_include)
env_64['TARGET_ARCH'] = 'x64'
env_64['SHLIBPREFIX'] = ''
env_64['SHLIBSUFFIX'] = '.dll'
env_64['OBJPREFIX'] = '../build/obj64/'
env_64['OBJSUFFIX'] = '.o'
env_64['PROGSUFFIX'] = '.dll'
env_64['CC'] = 'gcc'
env_64['CXX'] = 'g++'

# Create output directories
def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

ensure_dir('build/obj32')
ensure_dir('build/obj64')
# ensure_dir('build')

# Generate DEF file for exports
def generate_def_file(target, source, env):
    def_content = """LIBRARY %s
EXPORTS
    SecAcc_Init
    SecAcc_GenerateKey
    SecAcc_VerifyKey
""" % os.path.basename(str(target[0]))
    
    def_file = str(target[0]) + '.def'
    with open(def_file, 'w') as f:
        f.write(def_content)
    
    return 0

# Build targets
dll_32 = env_32.SharedLibrary(
    target='build/obj32/autosar_secacc_32',
    source=sources
)

env_32.AddPostAction(dll_32, generate_def_file)

dll_64 = env_64.SharedLibrary(
    target='build/obj64/autosar_secacc_64',
    source=sources
)

env_64.AddPostAction(dll_64, generate_def_file)

# Clean command
Clean('.', 'build')

# Command line options
AddOption('--w32', dest='build_32', action='store_true', default=False,
          help='Build 32-bit version')
AddOption('--w64', dest='build_64', action='store_true', default=False,
          help='Build 64-bit version')

# Select targets based on options
if GetOption('build_32'):
    Default(dll_32)
elif GetOption('build_64'):
    Default(dll_64)
else:
    Default(dll_32, dll_64)