type t = {
  name: string,
  title: string,
  avatarUrl: option(string),
  id: string,
};

let name = t => t.name;

let title = t => t.title;

let avatarUrl = t => t.avatarUrl;

let id = t => t.id;

let make = (~name, ~title, ~avatarUrl, ~id) => {name, title, avatarUrl, id};

let makeFromJs = coachData => {
  make(
    ~name=coachData##name,
    ~title=coachData##title,
    ~avatarUrl=coachData##avatarUrl,
    ~id=coachData##id,
  );
};