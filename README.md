# rCredits App

## Environment Setup

### Download the Repo

### Install Packages
```
cd [project]
npm install
```

## Configuration

1. Edit config.js
1. Run BUILD_TARGET=development gulp config

### Setup Sass
`ionic setup sass`

### Sometimes Needed
You may need to try these commands individually.

```
npm rebuild
gulp sass
npm install
bower install
```

### Run Unit tests
`karma start`

### Run
`ionic serve`

### Run on Device
1. `ionic state restore # Only if plugins or platforms may have changed`
1. `ionic run <android|ios>`
