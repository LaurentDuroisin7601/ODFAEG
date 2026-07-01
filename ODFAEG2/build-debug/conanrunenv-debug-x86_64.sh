script_folder="/home/laurent/ODFAEG-master/ODFAEG2/build-debug"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-debug-x86_64.sh"
for v in ALSA_CONFIG_DIR OPENSSL_MODULES
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-debug-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanrunenv-debug-x86_64.sh"
   fi
done

export ALSA_CONFIG_DIR="/home/laurent/.conan2/p/b/libalfe173d08b223d/p/res/alsa"
export OPENSSL_MODULES="/home/laurent/.conan2/p/b/opensb406191c489e5/p/lib/ossl-modules"