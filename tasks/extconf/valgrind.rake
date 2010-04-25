namespace :test do
  # partial-loads-ok and undef-value-errors necessary to ignore
  # spurious (and eminently ignorable) warnings from the ruby
  # interpreter
  VALGRIND_BASIC_OPTS = "--num-callers=30 --error-limit=no \
                         --partial-loads-ok=yes --undef-value-errors=no"

  desc "run test suite under valgrind with basic ruby options"
  task :valgrind => :compile do
    cmdline = "valgrind #{VALGRIND_BASIC_OPTS} ruby #{$hoe.make_test_cmd}"
    puts cmdline
    system cmdline
  end
end
