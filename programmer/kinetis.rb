require 'adiv5'
require 'memdev'

class Kinetis
  class FTFL
    include Peripheral

    register :FSTAT, 0x00 do
      # Flash Status Register, 0x00
      unsigned :CCIF, [0x00, 7], :desc => "Command Complete Interrupt"
      unsigned :RDCOLERR, [0x00, 6], :desc => "Read Collision Error"
      unsigned :ACCERR, [0x00, 5], :desc => "Access Error"
      unsigned :FPVIOL, [0x00, 4], :desc => "Protection Violation"
      unsigned :MGSTAT0, [0x00, 0], :desc => "Completion Status"

      # Flash Configuration Register, 0x01
      unsigned :CIE, [0x01, 7], :desc => "Command Complete Interrupt"
      unsigned :RDCOLLIE, [0x01, 6], :desc => "Read Collision Error Interrupt"
      unsigned :ERSAREQ, [0x01, 5], :desc => "Erase All Request"
      unsigned :ERSSUSP, [0x01, 4], :desc => "Erase Suspend"
      unsigned :PFLSH, [0x01, 2], :desc => "Flash memory configuration"
      unsigned :RAMRDY, [0x01, 1], :desc => "RAM Ready"
      unsigned :EEERDY, [0x01, 0], :desc => "EEPROM read ready"

      # Flash Security Register, 0x02
      unsigned :KEYEN, [0x02, 7..6], :desc => "Backdoor Key Security"
      unsigned :MEEN, [0x02, 5..4], :desc => "Mass Erase Enable Bits"
      unsigned :FSLACC, [0x02, 3..2], :desc => "Failure Analysis Access Code"
      unsigned :SEC, [0x02, 1..0], :desc => "Flash Security"

      # Flash Option Register, 0x03
      unsigned :OPT, [0x03, 7..0], :desc => "Nonvolatile Option"
    end

    register :FCCOB_Program_Section, 0x04 do
      unsigned :addr, [0, 23..0]
      unsigned :fcmd, [3, 7..0]
      unsigned :num_words, [6, 15..0]

      def initialize(addr, num_words)
        @fcmd = 0x0b
        @addr = addr
        @num_words = num_words
      end
    end

    # class Erase_Sector < BitStruct
    #   unsigned :addr, 24, :endian => :little
    #   unsigned :fcmd, 8

    #   def self.create(addr)
    #     c = self.new
    #     c.fcmd = 0x09
    #     c.addr = addr
    #     c
    #   end
    # end

    # class Set_FlexRAM < BitStruct
    #   unsigned :_rsvd, 16
    #   unsigned :function, 8
    #   unsigned :fcmd, 8

    #   def self.create(mode)
    #     c = self.new
    #     c.fcmd = 0x81
    #     case mode
    #     when :eeprom
    #       c.function = 0x00
    #     when :ram
    #       c.function = 0xff
    #     else
    #       raise RuntimeError, "invalid FlexRAM mode"
    #     end
    #     c
    #   end
    # end

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

    BaseAddress = 0x40020000

    def initialize(mem)
      @mem = mem
      @baseaddr = BaseAddress
      @devendian = :little
    end

    def FSTAT
      Regs1.new([self.read(FSTAT::Address)].pack("L")).FSTAT
    end

    def FCNFG
      Regs1.new([self.read(FSTAT::Address)].pack("L")).FCNFG
    end

    def cmd(fccob)
      raise RuntimeError, "FCCOB overflow" if fccob.size > 12

      # convert to 32 bit integers, fill potentially short ints
      fccob32 = (fccob + "\0"*3).unpack("L*")
      self.write(FCCOB::Address, fccob32)

      # clear all errors and start command
      regs = Regs1.new([self.read(FSTAT::Address)].pack("L"))
      regs.FSTAT.CCIF = 1
      regs.FSTAT.RDCOLERR = 1
      regs.FSTAT.ACCERR = 1
      regs.FSTAT.FPVIOL = 1
      self.write(FSTAT::Address, regs.unpack("L"))
      fstat = nil
      while self.FSTAT.CCIF == 0
        puts "waiting for completion"
        sleep 0.01
      end
      puts self.FSTAT.inspect_detailed
      raise RuntimeError, "error in flash execution" if self.FSTAT.MGSTAT0 != 0
      self
    end
  end

  class FlexRAM
    include MemDev

    BaseAddress = 0x14000000

    def initialize(mem)
      @mem = mem
      @baseaddr = BaseAddress
      @devendian = :little
    end
  end

  def initialize(*args)
    super
    @ftfl = Kinetis::FTFL.new(@dap)
    @flexram = Kinetis::FlexRAM.new(@dap)
    @sector_size = 1024
  end

  def program(data, addr)
    if String === data
      data = (data + "\0"*3).unpack('L*')
    end

    if data.size != @sector_size / 4 || (addr & (@sector_size - 1) != 0)
      raise RuntimeError, "invalid data size or alignment"
    end

    if @ftfl.FCNFG.RAMRDY != 1
      # set FlexRAM to RAM
      @ftfl.cmd(FTFL::FCCOB::Set_FlexRAM.create(:ram))
    end

    # @ftfl.cmd(FTFL::FCCOB::Erase_Sector.create(addr))

    # @flexram.write(0, data)
    # @flexram.read(0, :count => data.size)
    @adiv5.ap[0].write(0x1ffff000, data)
    @adiv5.ap[0].read(0x1ffff000, :count => data.size)

    # @ftfl.cmd(FTFL::FCCOB::Program_Section.create(addr, data.size))

    @adiv5.ap[0].read(addr, :count => data.size)
  end
end

if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  p.probe
  p.ap[0].write(0x20000000, 1)
  p.ap[0].read(0x20000000)
  p.ap[0].read(0x800)
  p.ap[0].read(0x4007e000)
  # k = Kinetis.new(p)
  # r = k.program("\xff"*1024, 0x800)
  # k = Kinetis::FTFL.new(p.ap[0])
  # puts k[0, :count => 6].map{|e| "%08x" % e}.join(" ")

  # stat = Kinetis::FTFL::Register.new(k[0, :count => 6].pack('L'))
  # puts stat.inspect_detailed
  # puts Kinetis::FTFL::Register.describe
end
