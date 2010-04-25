namespace :test do
  desc "run test suite under gdb"
  task :gdb => :compile do
    system "gdb --args ruby #{$hoe.make_test_cmd}"
  end
end
