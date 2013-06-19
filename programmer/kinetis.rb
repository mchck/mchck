require 'adiv5'
require 'armv7'
require 'register'

class Kinetis < ARMv7
  class FTFL
    include Peripheral

    default_address 0x40020000

    register :FSTAT, 0x00 do
      # Flash Status Register, 0x00
      bool :CCIF, [0x00, 7], :desc => "Command Complete Interrupt"
      bool :RDCOLERR, [0x00, 6], :desc => "Read Collision Error"
      bool :ACCERR, [0x00, 5], :desc => "Access Error"
      bool :FPVIOL, [0x00, 4], :desc => "Protection Violation"
      bool :MGSTAT0, [0x00, 0], :desc => "Completion Status"

      # Flash Configuration Register, 0x01
      bool :CIE, [0x01, 7], :desc => "Command Complete Interrupt"
      bool :RDCOLLIE, [0x01, 6], :desc => "Read Collision Error Interrupt"
      bool :ERSAREQ, [0x01, 5], :desc => "Erase All Request"
      bool :ERSSUSP, [0x01, 4], :desc => "Erase Suspend"
      bool :PFLSH, [0x01, 2], :desc => "Flash memory configuration"
      bool :RAMRDY, [0x01, 1], :desc => "RAM Ready"
      bool :EEERDY, [0x01, 0], :desc => "EEPROM read ready"

      # Flash Security Register, 0x02
      unsigned :KEYEN, [0x02, 7..6], :desc => "Backdoor Key Security"
      unsigned :MEEN, [0x02, 5..4], :desc => "Mass Erase Enable Bits"
      unsigned :FSLACC, [0x02, 3..2], :desc => "Failure Analysis Access Code"
      unsigned :SEC, [0x02, 1..0], :desc => "Flash Security"

      # Flash Option Register, 0x03
      # Defined in 6.3.3
      enum :NMI_DIS, [0x03, 2], {
        :disabled => 0,
        :enabled => 1
      }
      enum :EZPORT_DIS, [0x03, 1], {
        :disabled => 0,
        :enabled => 1
      }
      enum :LPBOOT, [0x03, 0], {
        :slow => 0,
        :fast => 1
      }
    end

    unsigned :FCCOB, 0x04, :vector => 3

    register :FCCOB_Program_Section, 0x04 do
      unsigned :addr, [0, 23..0]
      unsigned :fcmd, [3, 7..0]
      unsigned :num_words, [6, 15..0]

      def initialize(addr, num_words)
        super()
        self.fcmd = 0x0b
        self.addr = addr
        self.num_words = num_words
      end
    end

    register :FCCOB_Erase_Sector, 0x04 do
      unsigned :addr, [0, 23..0]
      unsigned :fcmd, [3, 7..0]

      def initialize(addr)
        super()
        self.fcmd = 0x09
        self.addr = addr
      end
    end

    register :FCCOB_Set_FlexRAM, 0x04 do
      enum :function, [2, 7..0], {
        :eeprom => 0x00,
        :ram => 0xff
      }
      unsigned :fcmd, [3, 7..0]

      def initialize(mode)
        super()
        self.fcmd = 0x81
        self.function = mode
      end
    end

    register :FPROT, 0x10 do
      unsigned :FPROT3, 7..0
      unsigned :FPROT2, 15..8
      unsigned :FPROT1, 23..16
      unsigned :FPROT0, 31..24
    end

    register :FEPROT, 0x14 do
      unsigned :EPROT, 23..16
      unsigned :DPROT, 31..24
    end

    def cmd(cmd_fccob)
      self.FCCOB = cmd_fccob

      # clear all errors and start command
      self.FSTAT.transact do |fstat|
        fstat.zero!
        fstat.CCIF = true
        fstat.RDCOLERR = true
        fstat.ACCERR = true
        fstat.FPVIOL = true
      end
      while !self.FSTAT.CCIF
        Log(:kinetis, 3){ "waiting for flash operation completion" }
        sleep 0.01
      end
      raise RuntimeError, "error in flash execution" if self.FSTAT.MGSTAT0
    end
  end

  class FlexRAM
    include Peripheral

    default_address 0x14000000
  end

  class Flash_Config_Field
    include Peripheral

    default_address 0x400

    # XXX would be nice to use the same register from above
    register :flash_config, 0x00 do
      unsigned :KEY0, [0x00, 31..0], :endian => :big
      unsigned :KEY1, [0x04, 31..0], :endian => :big
      unsigned :FPROT, [0x08, 31..0], :endian => :big
      enum :KEYEN, [0x0c, 7..6], {
        :disabled0 => 0b00,
        :disabled => 0b01,
        :enabled => 0b10,
        :disabled3 => 0b11
      }
      enum :MEEN, [0x0c, 5..4], {
        :enabled0 => 0b00,
        :enabled1 => 0b01,
        :disabled => 0b10,
        :enabled => 0b11
      }
      enum :FSLACC, [0x0c, 3..2], {
        :granted0 => 0b00,
        :denied1 => 0b01,
        :denied => 0b10,
        :granted => 0b11
      }
      enum :SEC, [0x0c, 1..0], {
        :secure0 => 0b00,
        :secure1 => 0b01,
        :unsecure => 0b10,
        :secure => 0b11
      }
      unsigned :FOPT, [0x0d, 7..0]
      unsigned :FEPROT, [0x0e, 7..0]
      unsigned :FDPROT, [0x0f, 7..0]
    end
  end

  def initialize(adiv5, magic_halt=false)
    super(adiv5)
    @mdmap = adiv5.ap(1)
    if !@mdmap || @mdmap.IDR.to_i != 0x001c0000
      raise RuntimeError, "not a Kinetis device"
    end

    begin
      if magic_halt
        # Kinetis hack: hold system & core in reset, so that flash and
        # security will have a chance to init, and we have a chance to
        # access the system.  If we don't hold the system in reset, it
        # might loop resetting itself.  While the system resets, it will
        # block debugger access until it has read the security bits.  If
        # the core loops in reset (e.g. because of empty flash), we will
        # get kicked in the nuts regularly.  Holding the system & core in
        # reset prevents this.
        # XXX hack
        Log(:kinetis, 1){ "holding system in reset" }
        # This waits until the system is in reset
        while @mdmap.read_ap(0) & 0b1110 != 0b0010
          @mdmap.write_ap(4, 0b11100)
        end
        # Now take system out of reset, but keep core in reset
        while @mdmap.read_ap(0) & 0b1110 != 0b1010
          @mdmap.write_ap(4, 0b10000)
        end

        self.enable_debug!
        self.catch_vector!(:CORERESET)

        # Now release the core from reset
        Log(:kinetis, 1){ "releasing core from reset" }
        @mdmap.write_ap(4, 0)

        # self.halt_core!
        # self.catch_vector!(:CORERESET, false)
        # self.disable_debug!
      end
      self.probe!
    rescue
      # If we were unsuccessful, maybe we need to do kinetis reset magic
      if !magic_halt
        Log(:kinetis, 1){ "trouble initializing, retrying with halt" }
        magic_halt = true
        retry
      else
        raise
      end
    end

    @ftfl = Kinetis::FTFL.new(@dap)
    @flexram = Kinetis::FlexRAM.new(@dap)
    @sector_size = 1024
  end

  def program_sector(addr, data)
    if String === data
      data = (data + "\0"*3).unpack('L*')
    end

    if data.size != @sector_size / 4 || (addr & (@sector_size - 1) != 0)
      raise RuntimeError, "invalid data size or alignment"
    end

    # Flash config field address
    if addr == 0x400
      fconfig = Flash_Config_Field.new(Peripheral::CachingProxy.new(data), 0).flash_config.dup
      if fconfig.SEC != :unsecure
        raise RuntimeError, "flash protection bits will brick device"
      end
    end

    if !@ftfl.FSTAT.RAMRDY
      # set FlexRAM to RAM
      @ftfl.cmd(FTFL::FCCOB_Set_FlexRAM.new(:ram))
    end

    @ftfl.cmd(FTFL::FCCOB_Erase_Sector.new(addr))
    @flexram.write(0, data)
    @ftfl.cmd(FTFL::FCCOB_Program_Section.new(addr, data.size))
  end

  def program(addr, data)
    if addr & (@sector_size - 1) != 0
      raise RuntimeError, "program needs to start on sector boundary"
    end

    if !self.core_halted?
      raise RuntimeError, "can only program flash when core is halted"
    end

    # pad data
    if data.bytesize % @sector_size != 0
      data += "\xff" * (@sector_size - data.bytesize % @sector_size)
    end

    datapos = 0
    while datapos < data.bytesize
      sectaddr = addr + datapos
      sector = data.byteslice(datapos, @sector_size)
      yield [sectaddr, datapos, data.bytesize]
      program_sector(sectaddr, sector)
      datapos += @sector_size
    end
  end
end

if $0 == __FILE__
  require 'backend-driver'
  adiv5 = Adiv5.new(BackendDriver.from_string(ARGV[0]))
  k = Kinetis.new(adiv5)
  # r = k.program_sector(0x800, "\xa5"*1024)
  # puts Log.hexary(adiv5.dap.read(0x800, :count => 1024/4))
end
