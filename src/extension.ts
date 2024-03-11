import * as vscode from "vscode";
import * as fs from "fs";
import * as path from "path";

export function activate(context: vscode.ExtensionContext) {
    let docsJson: any;
    const docsFilePath = path.join(context.extensionPath, "/docs/docs.json");
    readJSON(docsFilePath).then(data => {
        docsJson = data;
    });
    const keywordsFilePath = path.join(context.extensionPath, "/completions/keywords.txt");
    const keywords = fs.readFileSync(keywordsFilePath, "utf-8").split("\n");
    const functionsFilePath = path.join(context.extensionPath, "/completions/functions.txt");
    const functions = fs.readFileSync(functionsFilePath, "utf-8").split("\n");
    const language = "vex";
    const hoverProvider = vscode.languages.registerHoverProvider(language, {
        provideHover(document, position, token) {
            const wordRange = document.getWordRangeAtPosition(position);
            if (!wordRange) {
                return null;
            }
            const word = document.getText(wordRange);
            var nextChar = " ";
            var offset = 0;
            var maxOffset = 50;
            while (nextChar === " ") {
                nextChar = document.getText(new vscode.Range(position.line, wordRange.end.character + offset, position.line, wordRange.end.character + offset + 1));
                offset++;
                if (offset > maxOffset) {
                    break;
                }
            }
            if (nextChar === "(") {
                if (word && word in docsJson) {
                    const content = new vscode.MarkdownString(docsJson[word]);
                    content.supportHtml = true;
                    return new vscode.Hover(content);
                }
            }
        }
    });
    const completionProvider = vscode.languages.registerCompletionItemProvider("*", {
        provideCompletionItems(document, position, token, context) {
            const completionItems: vscode.CompletionItem[] = [];
            keywords.forEach(item => {
                completionItems.push(new vscode.CompletionItem(item.trim(), vscode.CompletionItemKind.Text));
            });
            functions.forEach(item => {
                let [label, insertText] = item.trim().split("#");
                completionItems.push({
                    label: label,
                    insertText: new vscode.SnippetString(insertText),
                    kind: vscode.CompletionItemKind.Snippet,
                    sortText: "z"
                });
            });
            return completionItems;
        }
    })
    context.subscriptions.push(hoverProvider, completionProvider);
}

function readJSON(filePath: string) {
    return new Promise((resolve, reject) => {
        fs.readFile(filePath, "utf8", (err, data) => {
            if (err) {
                reject(err);
            } else {
                try {
                    const jsonData = JSON.parse(data);
                    resolve(jsonData);
                } catch (error) {
                    reject(error);
                }
            }
        });
    });
}