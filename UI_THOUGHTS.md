# General API design:

`View` class represents a layout with children. Features `View::AddElement<T extends Element>` like `Actor::CreateComponent<T extends ActorComponent>`.

`View`s are also `Element`s, so you can nest views. Should be simple types of views for arranging children; like horizontal/vertical flex and absolute positioning.

Important `Element`s for MVP:

- `Button`
- `TextBlock`

Important `View` types:

- `FlexView`: Horizontal/Vertical options
- `CanvasView`: Allows positioning children relative to various anchor points (see prev implementation)



Adding a button to the top left of the screen:

```c++
auto view = engine->GetRootView();

auto flex = view->AddElement<FlexView>( /*horizontalAlignment*/ ALIGN_LEFT, /*verticalAlignment*/ ALIGN_TOP);
flex->SetPadding(10); // shorthand for 10 px left, top, right, and bottom padding

auto btn = flex->AddElement<Button>("Click Me!");
btn->AddClickHandler([=](){
    engine->SpawnActor<Cube>();
});

```

# Text Rendering

For text rendering I already have a high-quality antialiased font renderer for text in the 3D world, ideally find a way to adapt this for rendering UI text. See src/3d/text/*