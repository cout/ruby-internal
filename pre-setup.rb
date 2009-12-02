alias generate_files_dir_lib noop

def generate_files_dir_ext(rel)
  return unless extdir?(curr_srcdir())
  make 'generated_files' if File.file?('Makefile')
end

exec_task_traverse 'generate_files'
