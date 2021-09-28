if (!window.chrome)
  window.chrome = {}

if (!window.chrome.runtime) {
  window.chrome.runtime = {
    onConnect: {
      addListener: function(a,b,c) {
      },
    },
    id: "1"
  }
}