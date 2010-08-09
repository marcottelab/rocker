$:.unshift(File.dirname(__FILE__)) unless
  $:.include?(File.dirname(__FILE__)) || $:.include?(File.expand_path(File.dirname(__FILE__)))

# STDERR.puts("__FILE__=#{__FILE__}\ndirname=#{File.dirname(__FILE__)}")
# $:.unshift(File.join(File.dirname(__FILE__),'..','ext','rockerxx'))
# STDERR.puts($:)

require "rockerxx.so"
require "rubygems"
require "gnuplot"

module Rocker
  VERSION = '0.1.0'
  DBARGS  = "dbname=crossval_development user=jwoods password=youwish1"

  # Calculate AUCs in the current working directory
  def self.create(matrix_id, experiment_id)
    Rockerxx.new(DBARGS, matrix_id, experiment_id)
  end

  def self.cd matrix_id, exp_id, res_date
    Dir.chdir("/home/jwoods/NetBeansProjects/crossval/tmp/work/matrix_#{matrix_id}/experiment_#{exp_id}/results.#{res_date}/") do
      x = self.create(matrix_id, exp_id)
      yield x
    end
  end

  def self.roc_plot phenotype_id, threshold = 0.0
    result = nil
    self.cd do |rocker|
      result = rocker.calculate_plots phenotype_id, threshold
    end

    Gnuplot.open do |gp|
      Gnuplot::Plot.new(gp) do |plot|
        plot.title "ROC Plot of #{phenotype_id}, area #{result[:roc_area]}"
        plot.ylabel "True Positive Rate (Recall)"
        plot.xlabel "False Positive Rate"

        plot.data << Gnuplot::DataSet.new([result[:fpr_axis],result[:tpr_axis]]) do |ds|
          ds.with = "lines"
          ds.notitle
        end
      end
    end
  end

  def self.pr_plot phenotype_id, threshold = 0.0
    result = nil
    self.cd do |rocker|
      result = rocker.calculate_plots phenotype_id, threshold
    end

    Gnuplot.open do |gp|
      Gnuplot::Plot.new(gp) do |plot|
        plot.title "Precision-Recall Plot of #{phenotype_id}, area #{result[:pr_area]}"
        plot.ylabel "Precision"
        plot.xlabel "Recall"

        plot.data << Gnuplot::DataSet.new([result[:tpr_axis], result[:precision_axis]]) do |ds|
          ds.with = "linespoints"
          ds.notitle
        end
      end
    end
  end
end
