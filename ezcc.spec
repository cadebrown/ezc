# -*- mode: python -*-

block_cipher = None


a = Analysis(['ezcc.py', 'compiler_delegate.py', 'ezcc.py', 'EZcompiler.py', 'EZlogger.py', 'libBasic.py', 'lib_linker.py', 'libLoops.py', 'libMath.py', 'libPrint.py', 'parser.py', 'shared.py'],
             pathex=['/home/cade/projects/ezc'],
             binaries=None,
             datas=None,
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name='ezcc',
          debug=False,
          strip=False,
          upx=True,
          console=True )
