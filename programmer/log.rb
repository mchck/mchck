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

  def self.log(subsys, level, *args)
    if level <= @@levels[subsys]
      spc = "  " * (level - 1)
      $stderr.puts '%s%s: %s' % [spc, subsys.to_s.upcase, args.join(' ')]
    end
  end

  if ENV["DEBUG"]
      ENV["DEBUG"].split(/[,;]/).each do |d|
      lvls = d.split(/[:=]/, 2)
      self.level(*lvls)
    end
  end
end

def Log(*args)
  Log.log(*args)
end
