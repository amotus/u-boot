series="1"
name="imx8mn-var-som"
subject="Conversion of imx8mn-var-som/imx8mn-var-som-symphony to OF_UPSTREAM and fixes/improvements"

base_branch="denx/master"
compile_branch="denx/master"

commit_start="start ${name}-${series}"
commit_end="end ${name}-${series}"

# Additional CC entries (must be space separated):
cc_list="hugo@hugovil.com"

init_cmd="hvk-init.sh"
compile_cmd="hvk-compile.sh"

# Ignore warnings like ...IS_ENABLED(CONFIG...))' instead of '#if or #ifdef' where possible...
#ignore_checkpatch_errors="PREFER_IF"
ignore_checkpatch_errors="FILE_PATH_CHANGES"
