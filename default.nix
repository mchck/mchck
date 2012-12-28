{ version ? "0", revision ? "local" }:

with import <nixpkgs> {};
{
  crossSystem = {
    config = "arm-none-eabi";
    bigEndian = false;
    arch = "arm";
    float = "soft";
    #withTLS = true;
    #libc = "newlib";
    platform = {
      name = "mchck";
      #kernelMajor = "2.6";
      #kernelBaseConfig = "bcm63xx_defconfig";
      #kernelHeadersBaseConfig = "bcm63xx_defconfig";
      #uboot = null;
      #kernelArch = "mips";
      #kernelAutoModules = false;
      #kernelTarget = "vmlinux.bin";
    };
    #openssl.system = "linux-generic32";
    gcc = {
      arch = "arm";
    };
  };
}