class ::Installer
  remove_method :install_dir_ext

  # Hook to install shared objects in the right directory (the default
  # is to strip off the subdirectory)
  def install_dir_ext(rel)
    return unless extdir?(curr_srcdir())
    install_files rubyextentions('.'),
                  "#{config('sodir')}/#{rel}",
                  0555
  end
end
 
