variables:
    QT_FORMULAE: qt@5
    REPO_SLUG: $(Build.Repository.Name)
    CURRENT_COMMIT: $(Build.SourceVersion)

resources:
  repositories:
  - repository: scopy
    type: github
    endpoint: put_endpoint_here
    name: analogdevicesinc/scopy
    ref: pkg-man-test

trigger:
  branches:
    include:
    - main
  tags:
    include:
    - v*

pr:
  branches:
    include:
    - main

stages:
- stage: Builds
  jobs:
  - job: macOSBuilds
    strategy:
      matrix:
        macOS_13:
          vmImage: 'macOS-13'
          artifactName: 'macOS-13'
        macOS_14:
          vmImage: 'macOS-14'
          artifactName: 'macOS-14'
        macOS_15:
          vmImage: 'macOS-15'
          artifactName: 'macOS-15'

    pool:
      vmImage: $[ variables['vmImage'] ]
    steps:
    - checkout: self
      path: package
      fetchDepth: 0
      clean: true
    - checkout: scopy
      path: scopy
      fetchDepth: 0
      clean: true
    - script: |
            PKG_NAME=$(echo $(Build.Repository.Name) | awk -F "/" '{print $2}')
            echo "##vso[task.setvariable variable=PKG_NAME]$PKG_NAME"
      displayName: 'Extract Repository Name'
    - script: |
            mkdir -p scopy/packages/$(PKG_NAME)
            cp -r package/* scopy/packages/$(PKG_NAME)/
            echo "ls scopy/packages"
            ls scopy/packages
            echo "ls scopy/packages/$(PKG_NAME)"
            ls scopy/packages/$(PKG_NAME)/
      displayName: 'Copy $(PKG_NAME) to Scopy'
      workingDirectory: $(Build.Repository.LocalPath)/..
    - script: ./ci/macOS/install_macos_deps.sh
      displayName: 'Build and Install Dependencies'
      workingDirectory: $(Build.Repository.LocalPath)/../scopy
    - script: |
            pip3 install -r requirements.txt
            python3 ./package_generator.py -h
      displayName: 'Check package generator and install dependencies'
      workingDirectory: $(Build.Repository.LocalPath)/../scopy/tools/packagegenerator 
    - script: |
            export BUILD_HOST="$(vmImage)"
            export ACCOUNT_NAME=`echo $[ resources.repositories.scopy.name ] | awk -F "/" '{print $1}'`
            export PROJECT_NAME=`echo $[ resources.repositories.scopy.name ] | awk -F "/" '{print $2}'`
            export USERNAME="azure-pipelines"
            export BUILD_REPO_URL=$[ resources.repositories.scopy.url ]
            export RUN_ID=$(Build.BuildId)
            export RUN_NUMBER=$(Build.BuildNumber)
            export JOB_ID=$(System.JobId)
            export JOB_NAME=$(System.JobName)
            export RUNNER_ARCH=$(Agent.OSArchitecture)
            export MACOSX_DEPLOYMENT_TARGET=11.0
            ./ci/macOS/build_azure_macos.sh
      displayName: 'Build Scopy'
      workingDirectory: $(Build.Repository.LocalPath)/../scopy
    - script: |
            ls -R build
            cp build/Scopy.app/Contents/MacOS/Users/runner/work/1/scopy/build/packages/$(PKG_NAME)/plugins/*.dylib build/packages/$(PKG_NAME)/plugins
      displayName: 'Copy plugin library to the package'
      workingDirectory: $(Build.Repository.LocalPath)/../scopy  
    - script: |
            python3 ./package_generator.py -a --src=$(Build.Repository.LocalPath)/../scopy/build/packages/$(PKG_NAME) --dest=$(Build.ArtifactStagingDirectory)
      displayName: 'Create package zip'
      workingDirectory: $(Build.Repository.LocalPath)/../scopy/tools/packagegenerator
    - task: GithubRelease@1
      displayName: 'Push to continuous release'
      condition: and(succeeded(), and(ne(variables['Build.Reason'], 'PullRequest'), eq(variables['Build.SourceBranch'], 'refs/heads/master')))
      inputs:
        githubConnection: put_your_github_connection_here
        repositoryName: $(Build.Repository.Name)
        action: edit
        tag: continous
        assets: $(Build.ArtifactStagingDirectory)/*.zip
        assetUploadMode: replace
        isPreRelease: true
        addChangeLog: false
    - task: PublishPipelineArtifact@1
      displayName: 'Publish Package Artifact'
      inputs:
        targetPath: '$(Build.ArtifactStagingDirectory)'
        artifactName: '$(artifactName)'