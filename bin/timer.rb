#! /usr/bin/env ruby

module Timer
  # calling kill.sh from a bash script echo'd something undesirable that ruby swallows conveniently
  def kill_player
    system("#{ENV['HOME']}/series-timer/bin/kill.sh Audirvana")
  end

  MINUTE = 60
  TIME_EVERY = 10

  def wait(minutes, indent)
    print indent
    while minutes > 0 do
      sleep MINUTE
      minutes -= 1
      if (minutes % TIME_EVERY) == 0
        print Time.now.strftime("%R")
      else
        print "."
      end
      STDOUT.flush
    end
    puts
    puts
  end
  extend Timer
end

if __FILE__ == $0
  list = ARGV.empty? ? [1] : ARGV
  list.each_with_index do |arg, i|
    indent = "\t" * i
    puts "#{indent}#{i + 1}) #{arg} minutes"
    Timer.wait(arg.to_i, indent)
    Timer.kill_player
    system("#{ENV['HOME']}/series-timer/bin/say_time_and_play_random_tune.sh &")
  end
  indent = "\t" * list.size
  puts "#{indent}Last"
  system("sleep 5 && say Finished && #{ENV['HOME']}/series-timer/bin/say_time.sh")
  Timer.kill_player
end
