exports.config = {
  // set to "custom" instead of cucumber. 
  framework: 'custom',
 
  // path relative to the current config file 
  frameworkPath: require.resolve('../node_modules/protractor-cucumber')
};