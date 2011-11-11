$:.unshift(File.dirname(__FILE__)) unless
  $:.include?(File.dirname(__FILE__)) || $:.include?(File.expand_path(File.dirname(__FILE__)))

# STDERR.puts("__FILE__=#{__FILE__}\ndirname=#{File.dirname(__FILE__)}")
# $:.unshift(File.join(File.dirname(__FILE__),'..','ext','rockerxx'))
# STDERR.puts($:)

module Rocker
  begin
    require File.dirname(__FILE__) + "/../ext/rockerxx/rockerxx.so"
  rescue LoadError => e
    warn "Unable to load native extension (maybe still needs to be compiled?)"
  end
  # require "gnuplot"
  
  VERSION = '0.3.2'
  DBARGS  = "host=arrakis.icmb.utexas.edu dbname=crossval_development user=crossval password=youwish1"
  WORKDIR = "../crossval/tmp/work/"

  # Get database arguments from ActiveRecord if available. Otherwise, use the default.
  def self.db_args
    begin
      config = ActiveRecord::Base.connection.instance_variable_get(:@config)
      return "host=#{config[:host]} dbname=#{config[:database]} user=#{config[:username]} password=#{config[:password]}"
    rescue
      STDERR.puts("Warning: ActiveRecord not loaded or connection not instantiated. If you're running this from a Rails environment, that's a bug. Using default database settings.") unless @@warned
      @@warned = true
      return DBARGS
    end
  end
  
  # Calculate AUCs in the current working directory
  def self.create(matrix_id, experiment_id)
    Rockerxx.new(self.db_args, matrix_id, experiment_id)
  end

  def self.cd_results matrix_id, exp_id, res_date = "20100805210802"
    Dir.chdir(WORKDIR + "matrix_#{matrix_id}/experiment_#{exp_id}/results.#{res_date}/") do
      x = self.create(matrix_id, exp_id)
      yield x
    end
  end

  def self.cd_fold matrix_id, exp_id, fold_id
    Dir.chdir(WORKDIR + "matrix_#{matrix_id}/experiment_#{exp_id}/predictions#{fold_id}") do
      x = self.create(matrix_id, exp_id)
      yield x
    end
  end

  def self.plot matrix_id, exp_id, res_date_or_fold_id, phenotype_id, plot_type = :roc_plot, output_format = nil
    raise(ArgumentError, "Invalid plot type") unless plot_type == :roc_plot || plot_type == :pr_plot

    # Determine whether we're plotting all results or just one fold.
    sym = :results
    sym = :fold if res_date_or_fold_id.is_a?(Fixnum) && res_date_or_fold_id < 100
    result = nil
    self.send("cd_#{sym}", matrix_id, exp_id, res_date_or_fold_id) do |rocker|
      result = rocker.calculate_plots phenotype_id
    end

    self.send plot_type, result, phenotype_id, output_format
  end

  def self.most_recent_results_dir
    potential_results = []
    
    Dir.foreach(".") do |file|
      potential_results << file.split(".").last.to_i if file =~ /^results\.[0-9]*$/
    end

    "results.#{potential_results.sort.last}"
  end

  # Print a table of roc_area or pr_area from a given matrix/experiment, comparing
  # the sum of the quantities from each fold with the quantity from the combined
  # (sorted) results.
  #
  # This is for testing whether roc_areas can be treated as approximately additive.
  def self.print_partial_and_combined matrix_id, exp_id, folds = 5, sym = :roc_area
    r = Hash.new { |h,k| h[k] = {} }
    Dir.chdir(WORKDIR + "matrix_#{matrix_id}/experiment_#{exp_id}") do

      # First get the full roc_areas from the result directory. Use the most recent.
      results_dir = self.most_recent_results_dir
      Dir.chdir(results_dir) do
        rocker = self.create(matrix_id, exp_id)
        
        Dir.foreach(".") do |phenotype_id|
          next if phenotype_id == "." || phenotype_id == ".."
          r[phenotype_id.to_i][:full] = rocker.calculate_plots( phenotype_id.to_i )[sym]
        end
      end

      # Now get the roc_areas from each partial matrix.
      (0...folds).each do |fold|
        Dir.chdir("predictions#{fold}") do
          rocker = self.create(matrix_id, exp_id)

          Dir.foreach(".") do |phenotype_id|
            next if phenotype_id == "." || phenotype_id == ".."
            r[phenotype_id.to_i][:sum] ||= 0.0
            r[phenotype_id.to_i][:sum] += rocker.calculate_plots( phenotype_id.to_i )[sym]
          end
        end
      end

    end

    # Now print the results
    puts %w(PID full sum delta).join("\t")
    r.keys.sort.each do |phenotype_id|
      diff = (r[phenotype_id][:full] - r[phenotype_id][:sum]).abs
      puts "#{phenotype_id}\t#{r[phenotype_id][:full]}\t#{r[phenotype_id][:sum]}\t#{diff}"
    end

    nil
  end


protected
  def self.roc_plot result, phenotype_id, output_format = nil

    Gnuplot.open do |gp|
      Gnuplot::Plot.new(gp) do |plot|

        plot.term('svg size 600, 600') if output_format == :svg

        plot.title "ROC Plot of #{phenotype_id}, area #{result[:roc_area]}"
        plot.ylabel "True Positive Rate (Recall)"
        plot.xlabel "False Positive Rate"

        plot.data << Gnuplot::DataSet.new([result[:fpr_axis],result[:tpr_axis]]) do |ds|
          ds.with = "linespoints"
          ds.notitle
        end

        plot.output("output.svg") if output_format == :svg
      end
    end

    result
  end

  def self.pr_plot result, phenotype_id, output_format = nil

    Gnuplot.open do |gp|
      Gnuplot::Plot.new(gp) do |plot|

        plot.term('svg size 600, 600') if output_format == :svg

        plot.title "Precision-Recall Plot of #{phenotype_id}, area #{result[:pr_area]}"
        plot.ylabel "Precision"
        plot.xlabel "Recall"

        plot.data << Gnuplot::DataSet.new([result[:tpr_axis], result[:precision_axis]]) do |ds|
          ds.with = "linespoints"
          ds.notitle
        end

        plot.output("output.svg") if output_format == :svg
      end
    end

    result
  end
end
