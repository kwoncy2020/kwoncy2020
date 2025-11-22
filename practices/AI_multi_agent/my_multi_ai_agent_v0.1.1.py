import os, enum, asyncio, time, functools
from dataclasses import dataclass
from typing import List, Annotated, TypedDict, Dict, Tuple
from langgraph.graph import StateGraph, END
from langchain_ollama import ChatOllama, OllamaEmbeddings
from langchain_chroma import Chroma
from langchain_core.messages import HumanMessage, AIMessage, SystemMessage, BaseMessage
from langchain_core.documents import Document
import operator

class Local_sLM_Models(enum.Enum):
    phi4_mini: str = "phi4-mini:3.8b" # Q4_K_M 2.5GB
    gemma3_4b: str = "gemma3:4b" # Q4_K_M 3.3GB
    llama32_2b: str = "llama3.2:3b" # Q4_K_M 2GB
    qwen3_06b: str = "qwen3:0.6b" # Q4_K_M 523MB
    planner: str = gemma3_4b
    researcher:str = llama32_2b
    refiner:str = phi4_mini
    summarizer:str = phi4_mini
    

@dataclass
class Agent_Params:
    model: str
    temperature: float
    repeat_penalty: float
    top_p: float
    typical_p: float
    num_ctx: int = 4096



planner_params: Agent_Params = Agent_Params(**{
            "model": Local_sLM_Models.planner,     
            "temperature": 0.1,
            "repeat_penalty": 1.20,
            "top_p": 0.95,
            "typical_p": 0.98,
            "num_ctx": 4096
    })

researcher_params: Agent_Params = Agent_Params(**{
            "model": Local_sLM_Models.researcher,       
            "temperature": 0.3,
            "repeat_penalty": 1.18,
            "top_p": 0.95,
            "typical_p": 0.98,
            "num_ctx": 8192
    })

refiner_params: Agent_Params = Agent_Params(**{
            "model": Local_sLM_Models.refiner,      
            "temperature": 0.7,
            # "repeat_penalty": 1.05,
            "repeat_penalty": 1.15,
            "top_p": 0.96,
            # "typical_p": 0.90,
            "typical_p": 0.96,
            "num_ctx": 2048
    })

summarizer_params: Agent_Params = Agent_Params(**{
            "model": Local_sLM_Models.summarizer,     
            "temperature": 0.4,
            "repeat_penalty": 1.20,
            "top_p": 0.95,
            "typical_p": 0.98,
            "num_ctx": 8192
    })

# Todo: Get tokenizer using transformer for each model to get exact token length.
#       check compress history time and tokens not just node.
PERFORMENCE_LOG:list[dict] =[]
def node_time_check(task:str = ""): 
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            start_node_time = time.perf_counter()
            result = func(*args, **kwargs)
            duration_ms = round((time.perf_counter() - start_node_time) * 1000, 2)

            # tokenizer = get_tokenizer(task) # use decorator or func.__name__
            # output_text = kargs[state][0].content
            # tokens = tokenizer(output_text)
            # tokens = kargs[state][0].token_usage
            # tokens = kargs[state][0].usage_metadata.get("output_tokens",0)
            # ex) result.response_metadata  = {'model': 'gemma3:4b', ..., 'total_duration': 106069905400, 
            #               'load_duration': 4889952100, 'prompt_eval_count': 104, 
            #               'prompt_eval_duration': 1747559400, 'eval_count': 1343, 
            #               'eval_duration': 98324010600, 'logprobs': None, 
            #               'model_name': 'gemma3:4b', 'model_provider': 'ollama'}
            log_data = {
                    "node_name": func.__name__,
                    "duration_ms": f"{duration_ms}ms",
            }
            
            if (hasattr(result,"response_metadata")):
                log_data.update(result.response_metadata)
            elif isinstance(result,dict) and "response_metadata" in result:
                log_data.update(result["response_metadata"])

            PERFORMENCE_LOG.append(log_data)
            return result
        return wrapper
    return decorator

planner: ChatOllama = ChatOllama(**planner_params.__dict__)
researcher: ChatOllama = ChatOllama(**researcher_params.__dict__)
refiner: ChatOllama = ChatOllama(**refiner_params.__dict__)

# summarizer = ChatOllama(Local_sLM_Model_Params.summarizer) 
summarizer = refiner  # Todo: modify if resource limitation is changed.

# embeddings = OllamaEmbeddings(model="nomic-embed-text:v1.5")
embeddings = OllamaEmbeddings(model="embeddinggemma")  # Erase previous vector data with nomic-embed-text model.

user: str = "kwoncy"
topic:str = "base"  # Todo: make longterm memory from summarizer for better result every user. + make longterm memory arranger.
my_chroma_db_path = f"C:\\Users\\kwoncy\\.conda\\envs\\cy_agent_py310\\Lib\\site-packages\\chromadb\\my_conversations_history\\{user}"

if not os.path.exists(my_chroma_db_path):
    os.makedirs(my_chroma_db_path,exist_ok=True)
    docs = [
        Document(page_content="[test]: my docs practice test", metadata={"source": "example[test]"}),
        Document(page_content="[text]: my docs practice text", metadata={"source": "example[text]"}),
        Document(page_content="[longterm memory]: my docs practice longterm memory", metadata={"source": "example[longterm memory]"}),
    ]
    vectorstore = Chroma.from_documents(docs, embeddings, persist_directory=my_chroma_db_path)
else:
    vectorstore = Chroma(persist_directory=my_chroma_db_path, embedding_function=embeddings)

# retriever = vectorstore.as_retriever(search_kwargs={"k": 5})
retriever = vectorstore.as_retriever(search_type="similarity_score_threshold",
                                    search_kwargs={"k": 5, "score_threshold": 0.15},
                                    )

MAX_TURNS_BEFORE_SUMMARY: int = 10   
SUMMARY_TOKEN_THRESHOLD: int = 8192  
SAVE_RECENT_MESSAGE_NUM: int = 6

async def compress_history(messages: List[BaseMessage]) -> Tuple[List[BaseMessage], Dict]:
    # save the first system message + remain recent history + compress old history recursively
    # Todo: save summarized history as a longterm memory on vectorstore + rearrange longterm memory to remove duplacation.
    if len(messages) <= MAX_TURNS_BEFORE_SUMMARY:
        return messages
        
    recent = messages[-SAVE_RECENT_MESSAGE_NUM:] if len(messages) > SAVE_RECENT_MESSAGE_NUM else messages
    


    old_part = messages[:-SAVE_RECENT_MESSAGE_NUM]
    conversation_text = "\n".join([
        f"{'사용자' if isinstance(m, HumanMessage) else 'AI'}: {m.content}"
        for m in old_part
    ])
    
    summary_prompt = f"""다음 대화를 핵심만 4~6줄로 요약해줘.
    - 인사말, 반복, 감탄사는 완전히 제거해야함.
    - 중요한 사실, 사용자 의도, 선호사항, 현재 진행 주제 반드시 포함해야함.
    - 잘못대답한 사실, 사용자가 주제를 변경하거나 대화의 맥락이 바뀐다면 그 내용이 있다면 반드시 포함해야함.

    대화:
    {conversation_text}

    요약:"""
    
    result = summarizer.invoke(summary_prompt)
    summary = result.content.strip()
    
    # summarized + recent history
    new_history = [
        SystemMessage(content=f"[이전 대화 요약]:\n{summary}"),
    ] + recent
    
    if hasattr(result,"response_metadata"):
        return new_history,result.response_metadata
    elif isinstance(result,dict) and "response_metadata" in result:
        return new_history,result["response_metadata"]
    return new_history,{}


class AgentState(TypedDict):
    input: str
    messages: Annotated[List[BaseMessage], operator.add, "token range(2k~8k)"]
    response_metadata: dict 
    plan: str
    research: str
    refine: str
    summarize: str
    final_answer: str

@node_time_check()
def planner_node(state: AgentState):
    prompt = f"""사용자 질문: {state['input']}
    위 질문을 해결하기 위한 단계별 계획을 한국어로 작성해줘.
    필요하면 외부 지식 검색도 포함해."""
    result = planner.invoke(state["messages"] + [HumanMessage(content=prompt)])
    return {"plan": result.content, "messages": [result], "response_metadata":getattr(result,"response_metadata",{})}

@node_time_check()
def researcher_node(state: AgentState):
    # RAG 
    docs = retriever.invoke(state["plan"] or state["input"])
    prompt_head:str = ""
    if len(docs) == 0:
        prompt_head = ""
    else:
        context = "\n\n".join([d.page_content for d in docs])
        prompt_head = f"검색된 자료 또는 이전 대화내용: {context}\n"
    
    prompt = f"""{prompt_head}

    사용자 질문: {state['input']}
    계획: {state['plan']}

    위 자료를 바탕으로 정확한 답변을 준비해줘."""
    result = researcher.invoke(state["messages"] + [HumanMessage(content=prompt)])
    return {"research": result.content, "messages": [result], "response_metadata":getattr(result,"response_metadata",{})}

# def summarizer_node(state: AgentState):   Todo : modify if resource limitation changed.
@node_time_check()
def refiner_node(state: AgentState):

    current_messages = state["messages"]
    summarizer_medadata:dict ={}
    if len(current_messages) > MAX_TURNS_BEFORE_SUMMARY:
        [compressed,summarizer_medadata]= asyncio.run(compress_history(current_messages))
        state["messages"] = compressed  
    
    # {[m.content for m in state['messages'][-10:]]}
    prompt = f"""이전 대화 요약 및 최신 맥락:
    {[m.content for m in state['messages']]}

    연구 결과: {state['research']}

    위 내용을 바탕으로 사용자에게 친절하고 자연스러운 한국어로 최종 답변해줘."""
    result = refiner.invoke(state["messages"] + [HumanMessage(content=prompt)])
    return {"final_answer": result.content, "messages": [result], "response_metadata":getattr(result,"response_metadata",{}), "summarizer_medadata":summarizer_medadata}


workflow = StateGraph(AgentState)

workflow.add_node("planner", planner_node)
workflow.add_node("researcher", researcher_node)
workflow.add_node("refiner", refiner_node)

workflow.set_entry_point("planner")
workflow.add_edge("planner", "researcher")
workflow.add_edge("researcher", "refiner")
workflow.add_edge("refiner", END)

app = workflow.compile()


def ask(question: str):
    print(f"\n사용자: {question}\n")
    result = app.invoke({
        "input": question,
        "messages": [SystemMessage(content="너는 도움이 되는 한국어 AI 어시스턴트야.")],
        "planner": "",
        "research": "",
        "refiner":"",
        "final_answer": ""
    })
    print(f"AI: {result['final_answer']}\n")
    [print(log) for log in PERFORMENCE_LOG]
    return result


if __name__ == "__main__":
    
    print("put your question. 'quit' will end the conversation.\n")

    # topic_q = input("input your topic: ")

    while True:
        q = input("input your question: ")
        if q.lower() in ["quit", "exit"]:
            break
        ask(q)