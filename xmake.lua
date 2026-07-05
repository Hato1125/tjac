set_project('tjac')
set_languages('cxx26')

add_rules(
  'mode.debug',
  'mode.release',
  'mode.releasedbg'
)

if is_mode('releasedbg') then
  set_symbols('debug')
  set_strip('none')
end

target('libtjac')
  set_kind('static')
  add_files('src/*.cc')
  add_includedirs('src')

-- test/ と playground/ はローカル専用 (git管理外) なので、ある場合だけ定義する
if os.isdir(path.join(os.scriptdir(), 'test')) then
  target('test')
    set_kind('binary')
    set_default(false)
    add_files('test/*.cc')
    add_includedirs('src')
    add_deps('libtjac')
    add_tests('default')
end

-- 雑デバッグ用の遊び場。中身は自由に書き換えてよい
if os.isdir(path.join(os.scriptdir(), 'playground')) then
  target('playground')
    set_kind('binary')
    set_default(false)
    add_files('playground/*.cc')
    add_includedirs('src')
    add_deps('libtjac')
    add_links('SDL3')
end
