module MemDev
  class TransactProxy
    def initialize(mem, cache_reads)
      @mem = mem
      @writeset = {}
      @readset = {} if cache_reads
    end

    def read(addr, opt={})
      r = (opt[:count] || 1).times.map do |i|
        iaddr = addr + i * 4
        v = @writeset[iaddr]
        v ||= (@readset[iaddr] ||= @mem[iaddr]) if @readset
        v
      end
      if opt[:count]
        r
      else
        r[0]
      end
    end
    alias :[] :read

    def write(addr, val, opt={})
      if !val.respond.to? :each
        val = [val]
      end
      val.each_with_index do |v, i|
        @writeset[addr + i * 4] = v
      end
    end
    alias :[]= :write

    def commit!
      writes = @writeset.sort_by{|k,v| k}
      writes.each do |addr, val|
        @mem[addr] = val
      end
      @writeset = nil
    end
  end

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

  def transact(cache_reads=false)
    proxy = TransactProxy.new(self, cache_reads)
    yield proxy
    proxy.commit!
  end
end
