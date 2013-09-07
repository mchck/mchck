class Log
  @@levels = Hash.new(0)

  def self.level(*lvls)
    case lvls.count
    when 2
      sub = lvls.shift
      @@levels[sub.to_sym] = Integer(lvls.first)
    when 1
      @@levels.default = Integer(lvls.first)
    end
  end

  def self.[](sub)
    @@levels.fetch(sub.to_sym, nil)
  end

  def self.[]=(sub, lvl)
    @@levels[sub.to_sym] = Integer(lvl)
  end

  def self.log(subsys, level)
    if level <= @@levels[subsys]
      args = yield
      args = [args] unless args.respond_to? :join
      spc = "  " * (level - 1)
      $stderr.puts '%s%s: %s' % [spc, subsys.to_s.upcase, args.join(' ')]
    end
  end

  def self.hexary(ary)
    ary = [ary] unless ary.respond_to? :each
    ary.map do |e|
      "%08x" % e
    end.join(", ")
  end

  if ENV["DEBUG"]
      ENV["DEBUG"].split(/[,;]/).each do |d|
      lvls = d.split(/[:=]/, 2)
      self.level(*lvls)
    end
  end
end

def Log(subsys, level, &block)
  Log.log(subsys, level, &block)
end
