require File.dirname(__FILE__) + '/test_helper.rb'

class TestRocker < Test::Unit::TestCase

  def setup
    @mid = 1
    @xid = 259
    @rid = "20100805210802"
  end

  # Tests that all axes begin at 0 and end at 1 (except for the one in
  # precision axis that does not typically reach 0).
  def test_axis_ranges
    (1...295).each do |i|
      Rocker.cd(@mid, @xid, @rid) do |d|
        begin
          r = d.calculate_plots(i, 0.0)
        rescue ArgumentError => e
          STDERR.puts "Column #{i} not found, skipping"
          next
        end

        assert r[:tpr_axis].first == 0.0
        assert r[:tpr_axis].last == 1.0
        assert r[:fpr_axis].first == 0.0
        assert r[:fpr_axis].last == 1.0
        assert r[:precision_axis].first == 1.0
        # The last element of the precision axis should not be 0 -- should be
        # greater, but might be 0 if tp_axis.last is 0 (no known genes).
      end
    end
  end
end
