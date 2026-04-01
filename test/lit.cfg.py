import os

import lit.formats
import lit.util
from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst

config.name = 'EVMLIR'
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
config.suffixes = ['.mlir']
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.evmlir_obj_root, 'test')
config.excludes = ['CMakeLists.txt', 'lit.cfg.py', 'lit.site.cfg.py.in']

llvm_config.use_default_substitutions()
llvm_config.with_environment('PATH', config.evmlir_tools_dir, append_path=True)
llvm_config.with_environment('PATH', config.llvm_tools_dir, append_path=True)

config.substitutions.append(
    ('%evmlir-opt', os.path.join(config.evmlir_tools_dir, 'evmlir-opt'))
)
