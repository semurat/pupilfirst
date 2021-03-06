open CourseAuthors__Types;

let decodeProps = json =>
  Json.Decode.(
    json |> field("courseId", string),
    json |> field("authors", array(Author.decode)),
  );

let (courseId, authors) =
  DomUtils.parseJSONTag(~id="schools-courses-authors__props", ())
  |> decodeProps;

ReactDOMRe.renderToElementWithId(
  <CourseAuthors__Root courseId authors />,
  "schools-courses-authors__root",
);
