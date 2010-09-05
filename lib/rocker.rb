$:.unshift(File.dirname(__FILE__)) unless
  $:.include?(File.dirname(__FILE__)) || $:.include?(File.expand_path(File.dirname(__FILE__)))

# STDERR.puts("__FILE__=#{__FILE__}\ndirname=#{File.dirname(__FILE__)}")
# $:.unshift(File.join(File.dirname(__FILE__),'..','ext','rockerxx'))
# STDERR.puts($:)

module Rocker
  require "rockerxx.so"
  # require "gnuplot"
  
  VERSION = '0.2.1'
  DBARGS  = "host=arrakis.icmb.utexas.edu dbname=crossval_development user=crossval password=youwish1"

  # Get database arguments from ActiveRecord if available. Otherwise, use the default.
  def self.db_args
    begin
      config = ActiveRecord::Base.connection.instance_variable_get(:@config)
      return "host=#{config[:host]} dbname=#{config[:database]} user=#{config[:username]} password=#{config[:password]}"
    rescue
      STDERR.puts "Warning: ActiveRecord not loaded or connection not instantiated. If you're running this from a Rails environment, that's a bug. Using default database settings."
      return DBARGS
    end
  end
  
  # Calculate AUCs in the current working directory
  def self.create(matrix_id, experiment_id)
    Rockerxx.new(self.db_args, matrix_id, experiment_id)
  end

  def self.cd matrix_id = 1, exp_id = 259, res_date = "20100805210802"
    Dir.chdir("../crossval/tmp/work/matrix_#{matrix_id}/experiment_#{exp_id}/results.#{res_date}/") do
      x = self.create(matrix_id, exp_id)
      yield x
    end
  end

  def self.roc_plot matrix_id, exp_id, res_date, phenotype_id
    result = nil
    self.cd(matrix_id, exp_id, res_date) do |rocker|
      result = rocker.calculate_plots phenotype_id
    end

    Gnuplot.open do |gp|
      Gnuplot::Plot.new(gp) do |plot|
        plot.title "ROC Plot of #{phenotype_id}, area #{result[:roc_area]}"
        plot.ylabel "True Positive Rate (Recall)"
        plot.xlabel "False Positive Rate"

        plot.data << Gnuplot::DataSet.new([result[:fpr_axis],result[:tpr_axis]]) do |ds|
          ds.with = "linespoints"
          ds.notitle
        end
      end
    end

    result
  end

  def self.pr_plot matrix_id, exp_id, res_date, phenotype_id
    result = nil
    self.cd(matrix_id, exp_id, res_date) do |rocker|
      result = rocker.calculate_plots phenotype_id
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

    result
  end
end
