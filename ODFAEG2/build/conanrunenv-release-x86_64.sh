script_folder="/home/laurent/ODFAEG-master/ODFAEG2/build"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in ALSA_CONFIG_DIR OPENSSL_MODULES
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   fi
done

export ALSA_CONFIG_DIR="/home/laurent/.conan2/p/b/libal3843900a0a299/p/res/alsa"
export OPENSSL_MODULES="/home/laurent/.conan2/p/b/openscc873457aaabf/p/lib/ossl-modules"