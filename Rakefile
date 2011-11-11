#!/usr/bin/ruby
# -*- ruby -*-
# -*- coding: utf-8 -*-
$:.unshift(File.dirname(__FILE__)+'/lib/')

require 'rubygems'
require 'rdoc/task'
require 'rocker'
require 'hoe'

Hoe.plugin :git
Hoe.plugin :gemspec

EXT = "ext/rockerxx/rockerxx.so"

h = Hoe.spec 'rocker' do
  self.version = '0.3.2'
  self.require_ruby_version '>=1.8'
  self.developer('John Woods', 'john.woods@marcottelab.org')
  self.extra_deps = {'rice' => '>=1.4'}
  #s.requirements << 'libpqxx3'
  #s.requirements << 'rice-1.3.2'
  #s.requirements << 'gnuplot'
  # s.files = FileList['Rakefile', 'lib/rocker.rb', 'test/*.rb', 'ext/**/*.cpp', 'ext/**/*.h'].to_a
  self.spec_extras[:extensions] = "/ext/rockerxx/extconf.rb"
  self.clean_globs << EXT << "ext/rockerxx/*.o" << "ext/rockerxx/Makefile"
  self.description = <<EOF
Rocker is a C++-implemented Ruby module for ROC and precision-recall
calculations. It's designed to work with crossval's database, which is
probably not yet public; but it's fairly easily modifiable.
EOF
  self.need_rdoc = false
end
# spec.add_development_dependency('test_benchmark')

RDoc::Task.new(:docs) do |rd|
  rd.main = h.readme_file
  rd.options << '-d' if (`which dot` =~ /\/dot/) unless
    ENV['NODOT'] || Hoe::WINDOZE
  rd.rdoc_dir = 'doc'

  rd.rdoc_files.include("lib/**/*.rb")
  rd.rdoc_files += h.spec.extra_rdoc_files
  rd.rdoc_files.reject! {|f| f=="Manifest.txt"}
  title = h.spec.rdoc_options.grep(/^(-t|--title)=?$/).first
  if title then
    rd.options << title

    unless title =~ /\=/ then # for ['-t', 'title here']
    title_index = spec.rdoc_options.index(title)
    rd.options << spec.rdoc_options[title_index + 1]
    end
  else
    title = "#{h.name}-#{h.version} Documentation"
    title = "#{h.rubyforge_name}'s " + title if h.rubyforge_name != h.name
    rd.options << '--title' << title
  end
end

desc 'Publish rdocs with analytics support'
task :publish_docs => [:clean, :docs] do
  ruby %{aggregate_adsense_to_doc.rb}
  path = File.expand_path("~/.rubyforge/user-config.yml")
  config = YAML.load(File.read(path))
  host = "#{config["username"]}@rubyforge.org"

  remote_dir = "/var/www/gforge-projects/#{h.rubyforge_name}/#{h.remote_rdoc_dir
  }"
  local_dir = h.local_rdoc_dir
  Dir.glob(local_dir+"/**/*") {|file|
    sh %{chmod 755 #{file}}
  }
  sh %{rsync #{h.rsync_args} #{local_dir}/ #{host}:#{remote_dir}}
end

require 'rspec/core/rake_task'
namespace :spec do
  desc "Run all specs"
  RSpec::Core::RakeTask.new
  # options in .rspec in package root
end

#Rake::GemPackageTask.new(spec) do |pkg|
#  pkg.need_tar = true
#end

#Rake::ExtensionTask.new('rockerxx')

namespace :test do
  Rake::TestTask.new(:rocker) do |t|
    t.test_files = FileList['test/test_rocker.rb']
    t.warning = true
    t.verbose = true
  end
end

file EXT => ["ext/rockerxx/extconf.rb", "ext/rockerxx/rockerxx.cpp"] do
  Dir.chdir "ext/rockerxx" do
    ruby "extconf.rb"
    sh 'make'
    require "./#{EXT}"
  end
end

#namespace :test do
#  Rake::TestTask.new(:phenomatrix) do |t|
#    t.test_files = FileList['test/test_phenomatrix_extn.rb']
#    t.warning = true
#    t.verbose = true
#  end
#end

# TODO - want other tests/tasks run by default? Add them to the list
# remove_task :default
# task :default => [:spec, :features]

# vim: syntax=ruby
