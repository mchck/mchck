require 'adiv5'
require 'memdev'
require 'bit-struct'

module Kinetis
  class FTFL
    include MemDev

    class FSTAT < BitStruct
      unsigned :CCIF, 1, "Command Complete Interrupt"
      unsigned :RDCOLERR, 1, "Read Collision Error"
      unsigned :ACCERR, 1, "Access Error"
      unsigned :FPVIOL, 1, "Protection Violation"
      unsigned :Reserved, 3, "#Reserved"
      unsigned :MGSTAT0, 1, "Completion Status"

      note "Flash Status Register"
    end

    class FCNFG < BitStruct
      unsigned :CIE, 1, "Command Complete Interrupt"
      unsigned :RDCOLLIE, 1, "Read Collision Error Interrupt"
      unsigned :ERSAREQ, 1, "Erase All Request"
      unsigned :ERSSUSP, 1, "Erase Suspend"
      unsigned :Reserved, 1, "#Reserved"
      unsigned :PFLSH, 1, "Flash memory configuration"
      unsigned :RAMRDY, 1, "RAM Ready"
      unsigned :EEERDY, 1, "EEPROM read ready"

      note "Flash Configuration Register"
    end

    class FSEC < BitStruct
      unsigned :KEYEN, 2, "Backdoor Key Security"
      unsigned :MEEN, 2, "Mass Erase Enable Bits"
      unsigned :FSLACC, 2, "Failure Analysis Access Code"
      unsigned :SEC, 2, "Flash Security"

      note "Flash Security Register"
    end

    class FOPT < BitStruct
      unsigned :OPT, 8, "Nonvolatile Option"

      note "Flash Option Register"
    end

    class GenericFCCOB < BitStruct
      unsigned :CCOB, 8, "Register"

      note "Flash Common Command Object"
    end

    class FCCOB < BitStruct
      unsigned :FlashAddress2, 8, "Flash Address[7:0]" # FTFL_FCCOB3
      unsigned :FlashAddress1, 8, "Flash Address[15:8]" # FTFL_FCCOB2
      unsigned :FlashAddress0, 8, "Flash Address[23:16]" # FTFL_FCCOB1
      unsigned :FCMD, 8, "Flash Command" # FTFL_FCCOB0
      unsigned :DataByte3, 8, "Data Byte 3" # FTFL_FCCOB7
      unsigned :DataByte2, 8, "Data Byte 2" # FTFL_FCCOB6
      unsigned :DataByte1, 8, "Data Byte 1" # FTFL_FCCOB5
      unsigned :DataByte0, 8, "Data Byte 0" # FTFL_FCCOB4
      unsigned :DataByte7, 8, "Data Byte 7" # FTFL_FCCOBB
      unsigned :DataByte6, 8, "Data Byte 6" # FTFL_FCCOBA
      unsigned :DataByte5, 8, "Data Byte 5" # FTFL_FCCOB9
      unsigned :DataByte4, 8, "Data Byte 4" # FTFL_FCCOB8

      note "Flash Common Command Object"
    end

    class FPROT < BitStruct
      unsigned :FPROT3, 8, "PROT[7:0]"
      unsigned :FPROT2, 8, "PROT[15:8]"
      unsigned :FPROT1, 8, "PROT[23:16]"
      unsigned :FPROT0, 8, "PROT[31:24]"

      note "Flash Protection Register"
    end

    class FEPROT < BitStruct
      unsigned :EPROT, 8, "EEPROM Region Protect"

      note "EEPROM Protection Register"
    end

    class FDPROT < BitStruct
      unsigned :DPROT, 8, "Data Flash Region Protect"

      note "Data Flash Protection Register"
    end

    class Register < BitStruct
      nest :FSTAT, FSTAT, "FSTAT"
      nest :FCNFG, FCNFG, "FCNFG"
      nest :FSEC, FSEC, "FSEC"
      nest :FOPT, FOPT, "FOPT"
      nest :FCCOB, FCCOB, "FCCOB"
      nest :FPROT, FPROT, "FPROT"

      note "Register Descriptions"
    end

    BaseAddress = 0x40020000

    def initialize(mem)
      @mem = mem
      @baseaddr = BaseAddress
      @devendian = :little
    end
  end
end

if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  p.probe
  k = Kinetis::FTFL.new(p.ap[0])
  puts k[0, :count => 6].map{|e| "%08x" % e}.join(" ")

  stat = Kinetis::FTFL::Register.new(k[0, :count => 6].pack('L'))
  puts stat.inspect_detailed
  # puts Kinetis::FTFL::Register.describe
end
