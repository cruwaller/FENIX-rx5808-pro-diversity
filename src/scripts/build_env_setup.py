Import("env", "projenv")
import esp_ota
import esp_compress

platform = env.get('PIOPLATFORM', '')
target_name = env['PIOENV'].upper()

print("PLATFORM : '%s'" % platform)
print("BUILD ENV: '%s'" % target_name)

if platform in ['espressif8266']:
    env.AddPostAction("buildprog", esp_compress.compressFirmware)
    env.AddPreAction("${BUILD_DIR}/spiffs.bin",
                     [esp_compress.compress_files])
    env.AddPreAction("${BUILD_DIR}/${ESP8266_FS_IMAGE_NAME}.bin",
                     [esp_compress.compress_files])
    env.AddPostAction("${BUILD_DIR}/${ESP8266_FS_IMAGE_NAME}.bin",
                     [esp_compress.compress_fs_bin])
    if "_WIFI" in target_name:
        env.Replace(UPLOAD_PROTOCOL="custom")
        env.Replace(UPLOADCMD=esp_ota.on_upload)
elif platform in ['espressif32']:
    if "_WIFI" in target_name:
        env.Replace(UPLOAD_PROTOCOL="custom")
        env.Replace(UPLOADCMD=esp_ota.on_upload)
else:
    print("*** PLATFORM: '%s'" % platform)

