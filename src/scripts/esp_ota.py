import subprocess, os

def on_upload(source, target, env):
    upload_addr = ['fenix', 'fenix.local']

    firmware_path = str(source[0])
    bin_path = os.path.dirname(firmware_path)
    bin_target = os.path.join(bin_path, 'firmware.bin')
    if not os.path.exists(bin_target):
        raise Exception("No valid binary found!")

    cmd = ["curl", "--max-time", "60",
           "--retry", "2", "--retry-delay", "1",
           "-F", "data=@%s" % (bin_target,)]

    upload_port = env.get('UPLOAD_PORT', None)
    if upload_port is not None:
        upload_addr = [upload_port]

    for addr in upload_addr:
        addr = "http://%s/update" % (addr,)
        print(" ** UPLOADING TO: %s" % addr)
        try:
            subprocess.check_call(cmd + [addr])
            return
        except subprocess.CalledProcessError:
            print("FAILED!")

    raise Exception("WIFI upload FAILED!")
