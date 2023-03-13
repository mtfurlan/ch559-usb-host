# https://community.platformio.org/t/platformio-8051-model-medium-linker-err/18099/4
Import("env")
env.Append(LINKFLAGS=["--model-large"])
