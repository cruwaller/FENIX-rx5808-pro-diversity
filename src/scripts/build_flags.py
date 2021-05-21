Import("env")
import os
import hashlib
try:
    import git
except ImportError:
    env.Execute('"$PYTHONEXE" -m pip install GitPython')
    try:
        import git
    except ImportError:
        git = None


def parse_flags(path):
    if 'BUILD_FLAGS' not in env:
        env['BUILD_FLAGS'] = []
    build_flags = env['BUILD_FLAGS']
    try:
        with open(path, "r") as _f:
            for line in _f:
                define = line.strip()
                if define.startswith("-D"):
                    is_uid = "MY_PHRASE" in define or "MY_UID" in define

                    define_key = define.split("=")[0]
                    build_flags_copy = list(build_flags)
                    for flag in build_flags_copy:
                        if define_key in flag or (is_uid and ("MY_PHRASE" in flag or "MY_UID" in flag)):
                            ###print("remove %s (%s, %s)" % (flag, define_key, define))
                            # remove value and it will be replaced
                            build_flags.remove(flag)
                            break

                    if "MY_PHRASE" in define:
                        define = define.split("=")[1]
                        define = define.replace('"', '').replace("'", "")
                        key = define.replace("-D", "")
                        if len(define) < 8:
                            raise Exception("MY_PHRASE must be at least 8 characters long")
                        md5 = hashlib.md5(key.encode()).hexdigest()
                        print("Hash value: %s" % md5)
                        #my_uid = ["0x%02X"%ord(r) for r in md5[:6]]
                        my_uid = ["0x%02X"%int(md5[i:(i+2)],16) for i in range(0, 12, 2)]
                        define = "-DMY_UID=" + ",".join(my_uid)
                        print("Calculated UID[6] = {%s}" % ",".join(my_uid))
                    elif "MY_UID" in define and len(define.split(",")) != 6:
                        raise Exception("UID must be 6 bytes long")
                    build_flags.append(define)
    except IOError:
        return False
    return True

if not parse_flags("user_defines.txt"):
    err = "\n\033[91m[ERROR] File 'user_defines.txt' does not exist\n"
    raise Exception(err)
# try to parse user private params
parse_flags("user_defines_private.txt")

sha = None
if git:
    try:
        git_repo = git.Repo(
            os.path.abspath(os.path.join(os.getcwd(), os.pardir)),
            search_parent_directories=False)
        git_root = git_repo.git.rev_parse("--show-toplevel")
        fenix_repo = git.Repo(git_root)
        # git describe --match=NeVeRmAtCh --always --abbrev=6 --dirty
        hexsha = fenix_repo.git.describe('--dirty', '--abbrev=7', '--always', '--match=NeVeRmAtCh')
        print("hexsha: '%s'" % hexsha)
        if 'dirty' in hexsha:
            env['BUILD_FLAGS'].append("-DLATEST_COMMIT_DIRTY=1")
        #hexsha = fenix_repo.head.object.hexsha
        sha = ",".join(["0x%s" % x for x in hexsha[:7]])
    except git.InvalidGitRepositoryError:
        pass
if not sha:
    if os.path.exists("VERSION"):
        with open("VERSION") as _f:
            data = _f.readline()
            _f.close()
        hexsha = data.split()[1].strip()
        sha = ",".join(["0x%s" % x for x in hexsha[:6]])
    else:
        sha = "0,0,0,0,0,0"
print("Current SHA: %s" % sha)
env['BUILD_FLAGS'].append("-DLATEST_COMMIT="+sha)

print("\n[INFO] build flags: %s\n" % env['BUILD_FLAGS'])
