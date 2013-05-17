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
      unsigned :OPT, [0x03, 7..0], :desc => "Nonvolatile Option"
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
        Log :kinetis, 3, "waiting for flash operation completion"
        sleep 0.01
      end
      raise RuntimeError, "error in flash execution" if self.FSTAT.MGSTAT0
    end
  end

  class FlexRAM
    include Peripheral

    default_address 0x14000000
  end

  def initialize(adiv5, magic_halt=true)
    super(adiv5)
    @mdmap = adiv5.ap(1)
    if !@mdmap || @mdmap.IDR.to_i != 0x001c0000
      raise RuntimeError, "not a Kinetis device"
    end

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
      Log :kinetis, 1, "holding system in reset"
      # This waits until the system is in reset
      while @mdmap.read_ap(0) & 0b1110 != 0b0010
        @mdmap.write_ap(4, 0b11100)
      end
      # Now take system out of reset, but keep core in reset
      while @mdmap.read_ap(0) & 0b1110 != 0b1010
        @mdmap.write_ap(4, 0b10000)
      end
    end

    self.enable_debug!
    self.catch_vector!(:CORERESET)

    # Now release the core from reset
    Log :kinetis, 1, "releasing core from reset"
    @mdmap.write_ap(4, 0)

    self.halt_core!

    @ftfl = Kinetis::FTFL.new(@dap)
    @flexram = Kinetis::FlexRAM.new(@dap)
    @sector_size = 1024
  end

  def program_sector(data, addr)
    if String === data
      data = (data + "\0"*3).unpack('L*')
    end

    if data.size != @sector_size / 4 || (addr & (@sector_size - 1) != 0)
      raise RuntimeError, "invalid data size or alignment"
    end

    if true || !@ftfl.FSTAT.RAMRDY
      # set FlexRAM to RAM
      @ftfl.cmd(FTFL::FCCOB_Set_FlexRAM.new(:ram))
    end

    @ftfl.cmd(FTFL::FCCOB_Erase_Sector.new(addr))
    @flexram.write(0, data)
    @ftfl.cmd(FTFL::FCCOB_Program_Section.new(addr, data.size))

    @dap.read(addr, :count => data.size)
  end
end

if $0 == __FILE__
  adiv5 = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  k = Kinetis.new(adiv5)
  r = k.program_sector("\xa5"*1024, 0x800)
  puts Log.hexary(adiv5.dap.read(0x800, :count => 1024/4))
end
