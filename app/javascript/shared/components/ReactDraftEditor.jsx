import React from 'react';
import { Editor, EditorState, ContentState, Modifier } from 'draft-js';
import 'draft-js/dist/Draft.css';

const onChange = (onChangeCB, setEditorState, editorState) => {
  const text = editorState.getCurrentContent().getPlainText();
  onChangeCB(text);
  setEditorState(editorState)
};

const handleKeyCommand = (onChangeCB, editorState, setEditorState, command) => {
  switch (command) {
    case "bold":
      setBold(onChangeCB, editorState, setEditorState);
      return 'handled';
    case "italic":
      setItalic(onChangeCB, editorState, setEditorState);
      return 'handled';
    default:
      return 'not-handled';
  }
};

const updateSelection = (editorState, delimiter, filler, onChangeCB, setEditorState) => {
  const selectionState = editorState.getSelection();
  const anchorKey = selectionState.getAnchorKey();

  if (anchorKey !== selectionState.getFocusKey()) {
    return
  }

  const currentContent = editorState.getCurrentContent();
  const currentContentBlock = currentContent.getBlockForKey(anchorKey);
  const start = selectionState.getStartOffset();
  const end = selectionState.getEndOffset();
  const selectedText = currentContentBlock.getText().slice(start, end);

  var newContentState;

  if (selectionState.isCollapsed()) {
    newContentState = Modifier.insertText(currentContent, selectionState, delimiter + filler + delimiter)
  } else {
    newContentState = Modifier.replaceText(currentContent, selectionState, delimiter + selectedText + delimiter)
  }

  const newEditorState = EditorState.push(editorState, newContentState, 'insert-characters');
  onChange(onChangeCB, setEditorState, newEditorState);
}

const setBold = (onChangeCB, editorState, setEditorState) => {
  updateSelection(editorState, '**', 'bold text', onChangeCB, setEditorState);
}

const setItalic = (onChangeCB, editorState, setEditorState) => {
  updateSelection(editorState, '*', 'italicized text', onChangeCB, setEditorState);
}

const insertTextAtFocus = (editorState, insertText, onChangeCB, setEditorState) => {
  const selectionState = editorState.getSelection();

  let updatedSelectionState;

  /*
   * We need to collapse the selection to the focus (end of selection) in
   * case there is some selected text becase we don't want to modify any
   * existing text when inserted the supplied text.
   */
  if (selectionState.isCollapsed()) {
    updatedSelectionState = selectionState;
  } else {
    updatedSelectionState = selectionState.merge({
      anchorKey: selectionState.getFocusKey(),
      anchorOffset: selectionState.getFocusOffset(),
      isBackward: false
    });
  }

  const currentContent = editorState.getCurrentContent();
  const newContentState = Modifier.insertText(currentContent, updatedSelectionState, insertText);
  const newEditorState = EditorState.push(editorState, newContentState, 'insert-characters');

  onChange(onChangeCB, setEditorState, newEditorState);
};

export default function ReactDraftEditor(props) {
  const [editorState, setEditorState] = React.useState(() => {
    const contentState = ContentState.createFromText(props.content);
    return EditorState.createWithContent(contentState);
  }
  );

  const editor = React.useRef(null);

  function focusEditor() {
    editor.current.focus();
  }

  React.useEffect(() => {
    focusEditor()
  }, []);

  React.useEffect(() => {
    handleKeyCommand(props.onChange, editorState, setEditorState, props.command)
  }, [props.command, props.commandAt]);

  const contentChangedOutside = (props.content != editorState.getCurrentContent().getPlainText());

  React.useEffect(() => {
    if (contentChangedOutside) {
      const contentState = ContentState.createFromText(props.content)
      setEditorState(EditorState.createWithContent(contentState));
    }
  }, [contentChangedOutside]);

  React.useEffect(() => {
    if (typeof props.insertText === "string") {
      insertTextAtFocus(editorState, props.insertText, props.onChange, setEditorState)
    }
  }, [props.insertText])

  return (
    <div className="flex-grow px-3 pt-3" onClick={focusEditor}>
      <Editor
        ariaLabelledBy={props.ariaLabelledBy}
        ref={editor}
        placeholder={props.placeholder}
        handleKeyCommand={(command, editorState) => handleKeyCommand(props.onChange, editorState, setEditorState, command)}
        editorState={editorState}
        onChange={editorState => onChange(props.onChange, setEditorState, editorState)}
      />
    </div>
  );
};
