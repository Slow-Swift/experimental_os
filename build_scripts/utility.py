from decimal import Decimal
import re
import os

from SCons.Node.FS import Dir, File, Entry
from SCons.Environment import Environment

# Parse an integer size from a size with a suffix
def ParseSize(size: str):
    size_match = re.match(r'([0-9\.]+)([kmg]?)', size, re.IGNORECASE)
    if size_match is None:
        raise ValueError(f'Error: Invalud size {size}')

    result = Decimal(size_match.group(1))
    multiplier = size_match.group(2).lower()

    if multiplier == 'k':
        result *= 1024
    if multiplier == 'm':
        result *= 1024 ** 2
    if multiplier == 'g':
        result *= 1023 ** 3

    return int(result)

#
# Match a glob across an entire directory tree rather than just root directory
#
def GlobRecursive(env: Environment, pattern, node='.'):
    src = str(env.Dir(node).srcnode())
    cwd = str(env.Dir('.').srcnode())

    # Find all the directories in the tree
    dir_list = [src]
    for root, directories, _ in os.walk(src):
        for d in directories:
            dir_list.append(os.path.join(root, d))

    # Match the glob pattern for each of the directories
    globs = []
    for d in dir_list:
        glob = env.Glob(os.path.join(os.path.relpath(d, cwd), pattern))
        globs.append(glob)
    return globs

def FindIndex(_list, predicate):
    for i in range(len(_list)):
        if predicate(_list[i]):
            return i
    return None

def IsFileName(obj, name):
    if isinstance(obj, str):
        return name in obj
    elif isinstance(obj, File) or isinstance(obj, Dir) or isinstance(obj, Entry):
        return obj.name == name
    return False