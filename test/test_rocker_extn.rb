require "test/unit"

$:.unshift File.dirname(__FILE__) + "/../ext/rockerxx"
require "rockerxx.so"

class TestRockerExtn < Test::Unit::TestCase
  def test_working
    t = nil
    #Dir.chdir("../crossval/tmp/work/matrix_1/experiment_167/results.20100318222010/") do
      t = Rocker.calculate(1, 167)
    #end
    x = t.mean_auc
    puts "Mean AUC was #{x}"
  end

  def test_version_string
    puts Rocker::VERSION
  end

end
