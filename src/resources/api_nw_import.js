var nwNatives = requireNative('nw_natives');

apiBridge.registerCustomHook(function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;

  apiFunctions.setHandleRequest('selectProfile', function() {
      return bindingUtil.sendRequestSync('nw.Import.selectProfile', $Array.from(arguments), undefined, undefined)[0];
  });
  
});

//exports.binding = nw_binding.generate();
