# Adapted (mostly copied) from Nanobyte's video https://www.youtube.com/watch?v=ohT59ZfXo6k&list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN&index=18

from pathlib import Path
from SCons.Variables import *
from SCons.Environment import *
from SCons.Node import *
from build_scripts.phony_targets import PhonyTargets
from build_scripts.utility import ParseSize

# Load the build varaibles
VARS = Variables('build_scripts/config.py', ARGUMENTS)
VARS.AddVariables(
    EnumVariable(
        "config",
        help="Build configuration",
        default="debug",
        allowed_values=("debug", "release")),
    EnumVariable(
        "arch",
        help="Target architecture",
        default="i686",
        allowed_values=("i686")),
    EnumVariable(
        "imageFS",
        help="Type of image",
        default="fat32",
        allowed_values=("fat32")
    )
)
VARS.Add(
    "imageSize",
    help="The size of the image, will be rounded up to the rearest multiple of 512. " +
         "You can use suffixes (k/m/g).",
    default="128m",
    converter=ParseSize
)

# Not used yet
VARS.Add(
    "toolchain",
    help="Path to toolchain directory.",
    default="toolchain"
)

DEPS = {
    'binutils': '2.41',
    'gcc': '13.2.0'
}

#### Host Environment ####

HOST_ENVIRONMENT = Environment(variables=VARS,
    ENV = os.environ,
    AS = 'nasm',
    CFLAGS = ['-std=c99'],
    CXXFLAGS = ['-std=c++17'],
    CCFLAGS = ['-g'],
    STRIP = 'strip'
)

# Choose CC optimization level based on debug / release configuration
if HOST_ENVIRONMENT['config'] == 'debug':
    HOST_ENVIRONMENT.Append(CCFLAGS = ['-O0'])
else:
    HOST_ENVIRONMENT.Append(CCFLAGS = ['-O3'])


#### Target Environment ####

# The platform prefix is used for accessing the correct toolchain directories
# and for printing build info
platform_prefix = ''
if HOST_ENVIRONMENT['arch'] == 'i686':
    platform_prefix = 'i686-elf-'

# Setting up toolchain stuff now even though it won't be used until work on the kernel begins
# The toolchain folder is expected to have the following subfolders
#    toolchain/platform_prefix
#    toolchain/platform_prefix/bin
#    toolchain/platform_prefix/lib/gxx/platform_prefix
toolchainDir = Path(HOST_ENVIRONMENT['toolchain'], platform_prefix.removesuffix('-'))
toolchainBin = Path(toolchainDir, 'bin')
toolchainGccLibs = Path(toolchainDir, 'lib', 'gcc', platform_prefix.removesuffix('-'))

TARGET_ENVIRONMENT = HOST_ENVIRONMENT.Clone(
    AR = f'{platform_prefix}ar',
    CC = f'{platform_prefix}gcc',
    CXX = f'{platform_prefix}c++',
    LD = f'{platform_prefix}g++',
    RANLIB = f'{platform_prefix}ranlib',
    STRIP = f'{platform_prefix}strip',

    # Toolchain Info
    TOOLCHAIN_PREFIX = str(toolchainDir),
    TOOLCHAIN_LIBGCC = str(toolchainGccLibs),
    BINUTILS_URL = f'https://ftp.gnu.org/gnu/binutils/binutils-{DEPS["binutils"]}.tar.xz',
    GCC_URL = f'https://ftp.gnu.org/gnu/gcc/gcc-{DEPS["gcc"]}/gcc-{DEPS["gcc"]}.tar.xz'
)

# Add the necessary flags to the target environment commands
TARGET_ENVIRONMENT.Append(
    ASFLAGS = [
        '-f', 'elf',
        '-g'
    ],
    CCFLAGS = [
        '--ffreestanding',  # Generate freestanding code
        '-nostdlib'         # Don't include library references
    ],
    LINKFLAGS = [
        '-nostdlib'         # Don't link to a library
    ],

    # GCC comes with some header files that we can include even though we have not standard library
    LIBS = ['gcc'], 
    LIBPATH = [ str(toolchainGccLibs) ] # Where those header files can be found
)

# Add the toolchain binaries to the path
TARGET_ENVIRONMENT['ENV']['PATH'] += os.pathsep + str(toolchainBin)

# Finalize environment creation
Help(VARS.GenerateHelpText(HOST_ENVIRONMENT))
Export('HOST_ENVIRONMENT')
Export('TARGET_ENVIRONMENT')

# Locations to build to
variantDir = 'build/{0}_{1}'.format(TARGET_ENVIRONMENT['arch'], TARGET_ENVIRONMENT['config'])
variantDirStage1 = variantDir + '/stage1_{0}'.format(TARGET_ENVIRONMENT['imageFS'])

# Load the build stages
SConscript('src/bootloader/stage1/SConscript', variant_dir=variantDirStage1, duplicate=0)
SConscript('src/bootloader/stage2/SConscript', variant_dir=variantDir + "/stage2", duplicate=0)
SConscript('image/SConscript', variant_dir=variantDir, duplicate=0)

# Import image stage to we can build it
Import('image', 'stage1', 'stage2')
Default(stage1, stage2)
HOST_ENVIRONMENT.Alias('image', image)

# Create the phony targets
PhonyTargets(
    HOST_ENVIRONMENT, 
    run=['./scripts/run.sh', image[0].path],
    debug=['./scripts/debug.sh', image[0].path],
    bochs=['./scripts/bochs.sh', image[0].path],
    toolchain=['./scripts/setup_toolchain.sh', HOST_ENVIRONMENT['toolchain']]
)

# Link build dependencies
Depends('run', image)
Depends('debug', image)
Depends('bochs', image)