require 'adiv5'
require 'memdev'

module Kinetis
  class FTFL
    include MemDev

    FTFL = 0x40020000

    FSTAT = 0
    FCNFG = 1
    FSEC = 2
    FOPT = 3
    FCCOB3 = 4
    FCCOB7 = 8
    FCCOBB = 0xc
    FPROT3 = 0x10
    FEPROT = 0x16
    FDPROT = 0x17

    def initialize(mem)
      @mem = mem
      @baseaddr = FTFL
      @devendian = :big
    end
  end
end

if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  p.probe
  k = Kinetis::FTFL.new(p.ap[0])
  puts k[0, :count => 6].map{|e| "%08x" % e}.join(" ")
end
