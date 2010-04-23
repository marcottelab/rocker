require "test/unit"

$:.unshift File.dirname(__FILE__) + "/../ext/rocker"
require "rocker.so"

def database_string dbn = "crossval_development", u = "jwoods", p = "youwish1"
  "dbname=#{dbn} user=#{u} password=#{p}"
end

class TestRockerExtn < Test::Unit::TestCase
  def test_working
    t = Rocker.new(database_string, 1, 167)
    x = t.mean_auc
    puts "Mean AUC was #{x}"
  end

  def test_version_string
    puts Rocker::VERSION
  end

end
