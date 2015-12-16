(function(window) {

  'use strict';

  var Language = function() {
  };

  Language.Languages = [
    {
      code: 'en',
      name: 'lang.english'
    }
  ];

  Language.getLanguages = function() {
    return Language.Languages;
  };

  Language.DEFAULT_LANGUAGE = 'en';


  window.Language = Language;

}) (window);
