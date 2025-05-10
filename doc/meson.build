#install_man(
#'gmchess.6.in',
#'convert_pgn.6.in',
#'eleeye_engine.6.in',
#)

man_files = [
'gmchess.6',
'convert_pgn.6',
'eleeye_engine.6',
  ]

items = ['a', 'continue', 'b', 'break', 'c']
result = []
foreach i : man_files
  man_file = configure_file(
      input: i + '.in',
      output: i,
      copy: true,
  )
  install_man(man_file.full_path())
endforeach
