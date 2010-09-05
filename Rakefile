# File: Rakefile

require 'rubygems'
require 'rake/extensiontask'
require 'rake/testtask'

PKG = "rocker"
PKG_VERSION = [0,2,1]
AUTHOR = "John O. Woods, Marcotte Lab"
EMAIL = "john.woods@marcottelab.org"
HOMEPAGE = "http://github.com/MarcotteLabGit/fastknn"

spec = Gem::Specification.new do |s|
  s.platform = Gem::Platform::RUBY
  s.extensions = FileList["ext/**/extconf.rb"]
  s.summary = "Faster AUC calculation"
  s.name = PKG
  s.author = AUTHOR
  s.email = EMAIL
  s.homepage = HOMEPAGE
  s.version = PKG_VERSION.join('.')
  s.requirements << 'libpqxx3'
  s.requirements << 'rice-1.3.2'
  s.requirements << 'gnuplot'
  s.require_path = 'lib'
  # s.autorequire = 'rake'
  s.files = FileList['Rakefile', 'lib/rocker.rb', 'test/*.rb', 'ext/**/*.cpp', 'ext/**/*.h'].to_a
  s.description = <<EOF
Rocker is a C++-implemented Ruby module for ROC and precision-recall
calculations. It's designed to work with crossval's database, which is
probably not yet public; but it's fairly easily modifiable.
EOF
end
spec.add_development_dependency('test_benchmark')

Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar = true
end

Rake::ExtensionTask.new('rockerxx')

namespace :test do
  Rake::TestTask.new(:rocker) do |t|
    t.test_files = FileList['test/test_rocker.rb']
    t.warning = true
    t.verbose = true
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
