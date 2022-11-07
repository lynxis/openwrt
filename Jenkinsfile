import groovy.transform.Field

@Field
def base_image_version = 15

pipeline {
  agent {
    dockerfile {
      dir 'jenkins'
      additionalBuildArgs "--pull --build-arg BASE_IMAGE=samknows/toolchain-base:${base_image_version} --build-arg USER_ID=\$(id -u) --build-arg GROUP_ID=\$(id -g)"
      args  "-v ${env.WORKSPACE}:/opt/samknows/openwrt_skwb9p:rw,z"
      reuseNode true
      label 'toolchain_docker'
    }
  }

  options {
    disableConcurrentBuilds()
  }

  stages {
    stage('Prepare workspace') {
      steps {
        withCredentials([file(credentialsId: 'github_ssh_key', variable: 'KEY_FILE')]) {
          sh label:  'Set up GitHub SSH access',
             script: 'mkdir /home/builder/.ssh && cp ${KEY_FILE} /home/builder/.ssh/id_rsa && \
                      printf "Host github.com\n\tStrictHostKeyChecking no\n" > /home/builder/.ssh/config'
        }
      }
    }

    stage('Build') {
      steps {
        sh label:  'Get feeds',
           script: 'scripts/feeds update -a && scripts/feeds install -a && rm -r staging_dir tmp'

        sh label:  'Build',
           script: "chmod 400 files/etc/dropbear/authorized_keys && \
                    git checkout -- .config && make defconfig && \
                    make -j \$(getconf _NPROCESSORS_ONLN) -l \$(( 2 * \$(getconf _NPROCESSORS_ONLN) ))"
      }
    }

    stage('Repackage toolchain') {
      steps {
        sh label:  'Extract',
           script: 'mkdir -p repackaged_toolchain/opt/samknows/openwrt_skwb9p && \
                    cd repackaged_toolchain/opt/samknows/openwrt_skwb9p && \
                    tar --strip-components=1 -xf ../../../../bin/targets/*/*/openwrt-sdk-*.tar.xz'
        sh label:  'Compress',
           script: 'cd repackaged_toolchain && \
                    tar czf skwb9p_toolchain.tar.gz opt'
      }
    }
  }
}
