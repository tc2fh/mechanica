%{
    #include "rendering/NOMStyle.hpp"

%}

%include "NOMStyle.hpp"

%extend NOMStyle {
    %pythoncode %{
        @property
        def visible(self) -> bool:
            return self.getVisible()

        @visible.setter
        def visible(self, visible: bool):
            self.setVisible(visible)

        @property
        def colormap(self):
            return self.getColorMap()

        @colormap.setter
        def colormap(self, colormap: str):
            return self.setColorMap(colormap)
    %}
}

%pythoncode %{
    Style = NOMStyle
%}