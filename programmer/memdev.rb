module MemDev
  def read(addr, opt={})
    ret = @mem.read(@baseaddr + addr, opt)
    unless opt[:noswap]
      ret = bswap(ret)
    end
    ret
  end
  alias :[] :read

  def write(addr, val, opt = {})
    unless opt[:noswap]
      val = bswap(val)
    end
    @mem.write(@baseaddr + addr, val)
  end
  alias :[]= :write

  def bswap(val)
    if @devendian != @mem.endian
      if val.respond_to? :each
        d = val
      else
        d = [val]
      end
      d = d.pack('V*').unpack('N*')
      if !val.respond_to? :each
        d = d.first
      end
      d
    else
      val
    end
  end
end
