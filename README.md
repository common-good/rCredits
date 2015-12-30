# rCredits App

## Environment Setup & Refresh

### After Initial Repo Checkout

```
ionic setup sass
```

### After Pulling a New Branch or Commit

Update packages:

```
npm install
bower install
```

Then to update the app config, run:

```
BUILD_TARGET=xxx gulp config
```

where `xxx` is one of `development`, `staging`, or `production`. This command copies config from the `/config.js` and `/local_config.js` files. Only config matching the given build target (or lying outside an @if block) is copied.

### If Packages Get Screwed Up

`npm rebuild`

### If SCSS/CSS Gets Screwed Up

`gulp sass`

### To Run Unit Tests

`karma start`

### To Run in Browser

`ionic serve`

### To Run on Device

```
ionic state restore # Only if plugins or platforms may have changed
ionic run <android|ios>
```
