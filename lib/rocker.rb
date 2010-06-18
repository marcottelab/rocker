$:.unshift(File.dirname(__FILE__)) unless
  $:.include?(File.dirname(__FILE__)) || $:.include?(File.expand_path(File.dirname(__FILE__)))

# STDERR.puts("__FILE__=#{__FILE__}\ndirname=#{File.dirname(__FILE__)}")
# $:.unshift(File.join(File.dirname(__FILE__),'..','ext','rockerxx'))
# STDERR.puts($:)

require "rockerxx.so"

module Rocker
  VERSION = '0.0.11'
  DBARGS  = "dbname=crossval_development user=jwoods password=youwish1"

  # Calculate AUCs in the current working directory
  def self.calculate(matrix_id, experiment_id)
    Rockerxx.new(DBARGS, matrix_id, experiment_id)
  end
end
