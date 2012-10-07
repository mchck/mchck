class Log
  Levels = Hash.new(0).merge({
    :debug => 0,
    :info => 1,
    :notice => 2,
    :warning => 3,
    :error => 4,
  }).freeze

  @@level = Levels[:warning]

  case ENV["DEBUG"]
  when 1, 'y'
    @@level = Levels[:debug]
  end

  def self.level=(lvl)
    @@level = Levels[lvl]
  end

  def self.log(level, *args)
    if @@level <= Levels[level]
      $stderr.puts args.join(' ')
    end
  end
end

def Log(*args)
  Log.log(:info, *args)
end

def Debug(*args)
  Log.log(:debug, *args)
end
