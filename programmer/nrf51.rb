require 'adiv5'
require 'armv7'
require 'register'

class NRF51 < ARMv7  # not actually true, it's and armv6 cortex m0 device.
  def initialize(adiv5, magic_halt=false)
    super(adiv5)
    @mdmap = adiv5.ap(0)

    if !@mdmap || @mdmap.IDR.to_i != 0x4770021
    raise RuntimeError, "not an Nrf51 device"
    end
    cpuid = adiv5.dap.read(0xE000ED00)
    if cpuid != 0x410CC200
      raise RuntimeError, "not a Cortex M0"
    end

    @nvmc = NRF51::NVMC.new(@dap)
    @ficr = NRF51::FICR.new(@dap)

    @sector_size = @ficr.CODEPAGESIZE;

    self.probe!

  end



  class NVMC # Non-Volatile Memory Controller
    include Peripheral

    default_address 0x4001E000

    # from NRF51 Reference Manual p15.

    register :READY, 0x400 do
      bool :ready, [0x0,0], :desc => "NVMC is ready."
    end

    register :CONFIG, 0x504 do # Configuration register
      #A RW WEN Program memory access mode. It is strongly recommended to only activate erase and write modes when they are actively used.
      enum :WEN, [0x00, 1..0], {
         :REN => 0, # Read only access.
         :WEN => 1, # Write Enabled.
         :EEN => 2  # Erase enabled.
      }
    end


    unsigned :ERASEPAGE, 0x508 # Register for erasing a page in code region 1
    unsigned :ERASEPCR1, 0x508 # Register for erasing a page in code region 1. Equivalent to ERASEPAGE.
    unsigned :ERASEPCR0, 0x510 # Register for erasing a page in code region 0
    unsigned :ERASEALL,  0x50C # Register for erasing all non-volatile user memory.  Write 1 to erase all.
    unsigned :ERASEUICR, 0x514 # Register for erasing User Information Configuration Registers  // not available unless preprogramed factory code present.

    def mass_erase
      self.CONFIG.WEN = :EEN  # enable erase

      if self.CONFIG.WEN != :EEN
        raise RuntimeError, "can't set flash to erase"
      end

      self.ERASEALL = 1
      while !self.READY.ready
        Log(:nrf51, 1){ "waiting for flash erase completion" }
        sleep 0.01
      end

      self.CONFIG.WEN = :REN  # enable read

      if self.CONFIG.WEN != :REN
        raise RuntimeError, "can't set flash to read"
      end

    end

    def write(addr, data)
      self.CONFIG.WEN = :EEN

      if self.CONFIG.WEN != :EEN
        raise RuntimeError, "can't set flash to erase"
      end

      self.ERASEPAGE = addr
      while !self.READY.ready
        Log(:nrf51, 1){ "waiting for flash erase completion" }
        sleep 0.01
      end

      self.CONFIG.WEN = :WEN
      if self.CONFIG.WEN != :WEN
        raise RuntimeError, "can't set flash to write"
      end

      @backing.write(addr, data)

      while !self.READY.ready
        Log(:nrf51, 1){ "waiting for flash write completion" }
        sleep 0.01
      end

      self.CONFIG.WEN = :REN
      if self.CONFIG.WEN != :REN
        raise RuntimeError, "can't set flash to read"
      end

    end

  end

  class FICR  # Factory Information Configuration Registers
    include Peripheral

    # from NRF51 Reference Manual p18.

    default_address 0x10000000


    unsigned :CODEPAGESIZE, 0x010, :desc => "Code memory page size"
    unsigned :CODESIZE, 0x014, :desc => "Code memory size"
    unsigned :CLENR0, 0x028, :desc => "Length of code region 0 in bytes"
    unsigned :PPFC, 0x02C, :desc => "Pre-programmed factory code present"

    unsigned :NUMRAMBLOCK, 0x034, :desc => "Number of individually controllable RAM blocks"
    unsigned :SIZERAMBLOCK, 0x038, :vector => 4, :desc => "Size of RAM block n in bytes"

  end

  #0x10001000 UICR UICR User Information Configuration Registers

  def mass_erase

  end

  def program_sector(addr, data)
    if String === data
      data = (data + "\0"*3).unpack('L*')
    end

    if data.size != @sector_size / 4 || (addr & (@sector_size - 1) != 0)
      raise RuntimeError, "invalid data size or alignment"
    end

    @nvmc.write(addr, data)

  end

  def program(addr, data)
    super(addr, data, @sector_size)
  end

  def mmap_ranges
    ramsize = 0
    @ficr.NUMRAMBLOCK.times do |i|
      ramsize += @ficr.SIZERAMBLOCK[i]
    end

    flashsize = @ficr.CODESIZE * @ficr.CODEPAGESIZE

    Log(:nrf51, 1){ "#{@ficr.NUMRAMBLOCK} ram blocks, ramsize #{ramsize}, flashsize #{flashsize}" }
    super +
        [
            {:type => :flash, :start => 0, :length => flashsize, :blocksize => @sector_size},
            {:type => :ram, :start => 0x20000000, :length => ramsize}
        ]
  end

end



if $0 == __FILE__
  require 'backend-driver'
  adiv5 = Adiv5.new(BackendDriver.from_string(ARGV[0]))
  k = NRF51.new(adiv5)
  r = k.program_sector(0x00014000, "\xa5"*1024)
  puts Log.hexary(adiv5.dap.read(0x00014000, :count => 1024/4))
end
