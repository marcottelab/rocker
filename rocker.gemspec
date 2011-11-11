# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = "rocker"
  s.version = "0.3.2.20111111161102"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["John Woods"]
  s.date = "2011-11-11"
  s.description = "Rocker is a C++-implemented Ruby module for ROC and precision-recall\ncalculations. It's designed to work with crossval's database, which is\nprobably not yet public; but it's fairly easily modifiable.\n"
  s.email = ["john.woods@marcottelab.org"]
  s.extensions = ["/ext/rockerxx/extconf.rb"]
  s.extra_rdoc_files = ["History.txt", "Manifest.txt", "PostInstall.txt"]
  s.files = ["History.txt", "Manifest.txt", "PostInstall.txt", "README.rdoc", "Rakefile", "lib/rocker.rb", "script/console", "script/destroy", "script/generate", "test/test_helper.rb", "test/test_rocker.rb", "ext/rockerxx/extconf.rb", "ext/rockerxx/auc_info.h", "ext/rockerxx/constants.h", "ext/rockerxx/fetcher.h", "ext/rockerxx/updater.h", "ext/rockerxx/line_input_iterator.h", "ext/rockerxx/rockerxx.h", "ext/rockerxx/rockerxx.cpp", "test/test_rocker_extn.rb", "/ext/rockerxx/extconf.rb", ".gemtest"]
  s.homepage = "http://github.com/marcottelab/rocker"
  s.rdoc_options = ["--main", "README.txt"]
  s.require_paths = ["lib"]
  s.required_ruby_version = Gem::Requirement.new(">= 1.8")
  s.rubyforge_project = "rocker"
  s.rubygems_version = "1.8.10"
  s.summary = "This gem is fast C++ code for calculating AUCs on results of cross-validation"
  s.test_files = ["test/test_helper.rb", "test/test_rocker.rb", "test/test_rocker_extn.rb"]

  if s.respond_to? :specification_version then
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_runtime_dependency(%q<rice>, [">= 1.4"])
      s.add_development_dependency(%q<hoe>, ["~> 2.12"])
    else
      s.add_dependency(%q<rice>, [">= 1.4"])
      s.add_dependency(%q<hoe>, ["~> 2.12"])
    end
  else
    s.add_dependency(%q<rice>, [">= 1.4"])
    s.add_dependency(%q<hoe>, ["~> 2.12"])
  end
end
