(function(window) {

  var Seller = Class.create(User, {

    can: 0,
    descriptions: [],
    company: '',
    device: '',
    firstLogin: false
  });

  window.Seller = Seller;

}) (window);
