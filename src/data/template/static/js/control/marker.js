MarkerControl.prototype = new BaseControl("MarkerControl");

function MarkerControl() {
}

MarkerControl.prototype.create = function(wrapper) {
	var checkbox = document.createElement("input");
	checkbox.setAttribute("type", "checkbox");
	checkbox.setAttribute("checked", 1);
	checkbox.setAttribute("id", this.getName());
	checkbox.style.verticalAlign = "middle";
	var markerLayer = document.getElementsByClassName("leaflet-marker-pane")[0];
	var hiddenClass = "hidden";
	
	if(markerLayer) {
		checkbox.addEventListener("change", function() {
			if(checkbox.checked) {
				var classNames = markerLayer.className.split(" ");
				var classIndex = classNames.indexOf(hiddenClass);
				if(classIndex >= 0) {
					classNames.splice(classIndex, 1);
					markerLayer.className = classNames.join(" ");
				}
			} else {
				markerLayer.className += " " + hiddenClass;
			}
		});
	}
	
	var text = document.createElement("label");
	text.setAttribute("for", this.getName());
	text.innerHTML = "Show markers: ";
	
	wrapper.appendChild(text);
	wrapper.appendChild(checkbox);
};
